#ifndef PBRT_CORE_MIPMAP_H
#define PBRT_CORE_MIPMAP_H

#include "pbrt.h"
#include "memory.h"
#include "geometry.h"
#include "texture.h"
#include "spectrum.h"
#include "parallel.h"

namespace pbrt
{
	enum class ImageWrap { Repeat, Black, Clamp };
	struct ResampleWeight
	{
		int firstTexel;
		float weight[4];
	};
	template <typename T>
	class MIPMap
	{
	public:
		MIPMap(const Point2i& res, const T* img, bool doTrilinear = false, float maxAnisotropy = 8.f, ImageWrap wrapMode = ImageWrap::Repeat);
		int Width() const { return resolution.x; }
		int Height() const { return resolution.y; }
		int Levels() const { return pyramid.size(); }
		const T& Texel(int level, int s, int t) const;
		T Lookup(const Point2f& st, float width = 0.f) const;
        T Lookup(const Point2f &st, Vector2f dstdx, Vector2f dstdy) const;
        T EWA(int level, Point2f st, Vector2f dst0, Vector2f dst1) const;
		T triangle(int level, const Point2f& st) const;
	private:
		const bool doTrilinear;
		const float maxAnisotropy;
		const ImageWrap wrapMode;
		Point2i resolution;
		std::vector<std::unique_ptr<BlockedArray<T>>> pyramid;
		std::unique_ptr<ResampleWeight[]> resampleWeights(int oldRes, int newRes);

        static constexpr int WeightLUTSize = 128;
        static float weightLut[WeightLUTSize];
	};

    template <typename T>
    MIPMap<T>::MIPMap(const Point2i& res, const T* img, bool doTrilinear, float maxAnisotropy, ImageWrap wrapMode): doTrilinear(doTrilinear), maxAnisotropy(maxAnisotropy), wrapMode(wrapMode), resolution(res)
    {
        std::unique_ptr<T[]> resampledImage = nullptr;
        if(!IsPowerOf2(resolution.x) || !IsPowerOf2(resolution.y))
        {
            Point2i resPow2(RoundUpPow2(resolution.x), RoundUpPow2(resolution.y));
            std::unique_ptr<ResampleWeight[]> sWeights =
                    resampleWeights(resolution.x, resPow2.x);
            std::unique_ptr<T[]> sResampledImage(new T [resPow2.x * resolution.y]);
            resampledImage.reset(new T[resPow2.x * resPow2.y]);
            ParallelFor([&](int t)
                        {
                            for (int s = 0; s < resPow2.x; ++s)
                            {
                                sResampledImage[t * resPow2.x + s] = 0.f;
                                for (int j = 0; j < 4; ++j)
                                {
                                    int origS = sWeights[s].firstTexel + j;
                                    if (wrapMode == ImageWrap::Repeat)
                                        origS = Mod(origS, resolution.x);
                                    if (wrapMode == ImageWrap::Clamp)
                                        origS = Clamp(origS, 0, resolution.x - 1);
                                    if (origS >= 0 && origS < (int)resolution.x)
                                        sResampledImage[t * resPow2.x + s] +=
                                                sWeights[s].weight[j] * img[t * resolution.x + origS];
                                }
                            }
                        }, resolution.y, 16);
            std::unique_ptr<ResampleWeight[]> tWeights =
                    resampleWeights(resolution.y, resPow2.y);
            ParallelFor([&](int s)
                        {
                            for (int t = 0; t < resPow2.y; ++t)
                            {
                                resampledImage[t * resPow2.x + s] = 0.f;
                                for (int j = 0; j < 4; ++j)
                                {
                                    int offset = tWeights[t].firstTexel + j;
                                    if (wrapMode == ImageWrap::Repeat)
                                        offset = Mod(offset, resolution[1]);
                                    else if (wrapMode == ImageWrap::Clamp)
                                        offset = Clamp(offset, 0, resolution.y - 1);
                                    if (offset >= 0 && offset < resolution.y)
                                    {
                                        resampledImage[t * resPow2.x + s] +=
                                                tWeights[s].weight[j] * sResampledImage[s * resolution.y + offset];
                                    }
                                }
                            }
                        }, resPow2.x, 16);
            resolution = resPow2;
        }

        int nLevels = 1 + Log2Int(std::max(resolution.x, resolution.y));
        pyramid.resize(nLevels);
        pyramid[0].reset(new BlockedArray<T>(resolution.x, resolution.y,
                                             resampledImage? resampledImage.get() : img));
        for(int i = 1; i < nLevels; ++i)
        {
            int sRes = std::max(1, pyramid[i - 1]->uSize() / 2);
            int tRes = std::max(1, pyramid[i - 1]->vSize() / 2);
            pyramid[i].reset(new BlockedArray<T>(sRes, tRes));
            ParallelFor([&](int t)
                        {
                            for (int s = 0; s < sRes; ++s)
                                (*pyramid[i])(s, t) = .25f *
                                                      (Texel(i - 1, 2 * s, 2 * t) + Texel(i - 1, 2 * s + 1, 2 * t) +
                                                       Texel(i - 1, 2 * s, 2 * t + 1) + Texel(i - 1, 2 * s + 1, 2 * t + 1));
                        }, tRes, 16);
        }
    }

    template <typename T>
    const T& pbrt::MIPMap<T>::Texel(int level, int s, int t) const
    {
        const BlockedArray<T>& l = *pyramid[level];
        switch (wrapMode)
        {
            case ImageWrap::Repeat:
                s = Mod(s, l.uSize());
                t = Mod(t, l.vSize());
                break;
            case ImageWrap::Clamp:
                s = Clamp(s, 0, l.uSize() - 1);
                t = Clamp(t, 0, l.vSize() - 1);
                break;
            case ImageWrap::Black:
                static const T black = 0.f;
                if (s < 0 || s >= (int)l.uSize() || t < 0 || t >= (int)l.vSize())
                    return black;
                break;
        }
        return l(s, t);
    }

    template<typename T>
    T MIPMap<T>::Lookup(const Point2f& st, float width) const
    {
        float level = Levels() - 1 + Log2(std::max(width, (float)1e-8));
        if (level < 0)
            return triangle(0, st);
        else if (level >= Levels() - 1)
            return Texel(Levels() - 1, 0, 0);
        else
        {
            int iLevel = std::floor(level);
            float delta = level - iLevel;
            return Lerp(delta, triangle(iLevel, st), triangle(iLevel + 1, st));
        }
    }


    template<typename T>
    T MIPMap<T>::Lookup(const Point2f &st, Vector2f dst0, Vector2f dst1) const
    {
        if (doTrilinear) {
            float width = std::max(std::max(std::abs(dst0[0]), std::abs(dst0[1])),
                                   std::max(std::abs(dst1[0]), std::abs(dst1[1])));
            return Lookup(st, width);
        }
        //++nEWALookups;
        //ProfilePhase p(Prof::TexFiltEWA);
        // Compute ellipse minor and major axes
        if (dst0.LengthSquared() < dst1.LengthSquared()) std::swap(dst0, dst1);
        float majorLength = dst0.Length();
        float minorLength = dst1.Length();

        // Clamp ellipse eccentricity if too large
        if (minorLength * maxAnisotropy < majorLength && minorLength > 0) {
            float scale = majorLength / (minorLength * maxAnisotropy);
            dst1 *= scale;
            minorLength *= scale;
        }
        if (minorLength == 0) return triangle(0, st);

        // Choose level of detail for EWA lookup and perform EWA filtering
        float lod = std::max((float)0, Levels() - (float)1 + Log2(minorLength));
        int ilod = std::floor(lod);
        return Lerp(lod - ilod, EWA(ilod, st, dst0, dst1),
                    EWA(ilod + 1, st, dst0, dst1));
    }


    template <typename T>
    T MIPMap<T>::EWA(int level, Point2f st, Vector2f dst0, Vector2f dst1) const {
        if (level >= Levels()) return Texel(Levels() - 1, 0, 0);
        // Convert EWA coordinates to appropriate scale for level
        st[0] = st[0] * pyramid[level]->uSize() - 0.5f;
        st[1] = st[1] * pyramid[level]->vSize() - 0.5f;
        dst0[0] *= pyramid[level]->uSize();
        dst0[1] *= pyramid[level]->vSize();
        dst1[0] *= pyramid[level]->uSize();
        dst1[1] *= pyramid[level]->vSize();

        // Compute ellipse coefficients to bound EWA filter region
        float A = dst0[1] * dst0[1] + dst1[1] * dst1[1] + 1;
        float B = -2 * (dst0[0] * dst0[1] + dst1[0] * dst1[1]);
        float C = dst0[0] * dst0[0] + dst1[0] * dst1[0] + 1;
        float invF = 1 / (A * C - B * B * 0.25f);
        A *= invF;
        B *= invF;
        C *= invF;

        // Compute the ellipse's $(s,t)$ bounding box in texture space
        float det = -B * B + 4 * A * C;
        float invDet = 1 / det;
        float uSqrt = std::sqrt(det * C), vSqrt = std::sqrt(A * det);
        int s0 = std::ceil(st[0] - 2 * invDet * uSqrt);
        int s1 = std::floor(st[0] + 2 * invDet * uSqrt);
        int t0 = std::ceil(st[1] - 2 * invDet * vSqrt);
        int t1 = std::floor(st[1] + 2 * invDet * vSqrt);

        // Scan over ellipse bound and compute quadratic equation
        T sum(0.f);
        float sumWts = 0;
        for (int it = t0; it <= t1; ++it) {
            float tt = it - st[1];
            for (int is = s0; is <= s1; ++is) {
                float ss = is - st[0];
                // Compute squared radius and filter texel if inside ellipse
                float r2 = A * ss * ss + B * ss * tt + C * tt * tt;
                if (r2 < 1) {
                    int index =
                            std::min((int)(r2 * WeightLUTSize), WeightLUTSize - 1);
                    float weight = weightLut[index];
                    sum += Texel(level, is, it) * weight;
                    sumWts += weight;
                }
            }
        }
        return sum / sumWts;
    }

    template<typename T>
    T MIPMap<T>::triangle(int level, const Point2f& st) const
    {
        level = Clamp(level, 0, Levels() - 1);
        float s = st.x * pyramid[level]->uSize() - .5f;
        float t = st.y * pyramid[level]->vSize() - .5f;
        int s0 = std::floor(s), t0 = std::floor(t);
        float ds = s - s0, dt = t - t0;
        return (1 - ds) * (1 - dt) * Texel(level, s0, t0) +
               (1 - ds) * dt * Texel(level, s0, t0 + 1) +
               ds * (1 - dt) * Texel(level, s0 + 1, t0) +
               ds * dt * Texel(level, s0 + 1, t0 + 1);
    }

    template <typename T>
    std::unique_ptr<ResampleWeight[]> MIPMap<T>::resampleWeights(int oldRes, int newRes)
    {
        assert(newRes > oldRes);
        std::unique_ptr<ResampleWeight[]> wt(new ResampleWeight[newRes]);
        float filterWidth = 2.f;
        for(int i = 0; i < newRes; ++i)
        {
            float center = (i + .5f) * oldRes / newRes;
            wt[i].firstTexel = std::floor((center - filterWidth) + .5f);
            for(int j = 0; j < 4; ++j)
            {
                float pos = wt[i].firstTexel + j + .5f;
                // same value in same relative position, maybe use map cache it?
                wt[i].weight[j] = Lanczos((pos - center) / filterWidth);
            }
            float invSumWts = 1 / (wt[i].weight[0] + wt[i].weight[1] +
                                   wt[i].weight[2] + wt[i].weight[3]);
            for (int j = 0; j < 4; ++j)
                wt[i].weight[j] *= invSumWts;
        }
        return wt;
    }

    template <typename T>
    float MIPMap<T>::weightLut[WeightLUTSize];
}

#endif