#include "mipmap.h"
#include "spectrum.h"

namespace pbrt
{
	template <typename T>
	MIPMap<T>::MIPMap(const Point2i& res, const T* img, bool doTrilinear, float maxAnisotropy, ImageWrap wrapMode): doTrilinear(doTrilinear), maxAnisotropy(maxAnisotropy), wrapMode(wrapMode), resolution(res)
	{
		std::unique_ptr<T[]> resampledImage = nullptr;
		if(!IsPowerOf2(resolution.x) || !IsPowerOf2(resolution.y))
		{
			Point2i resPow2(RoundUpPow2(resolution.x), RoundUpPow2(resolution.y));
			std::unique_ptr<ResampleWeight[]> sWeights =
				resampleWeights(resolution.x, resPow2.x);
			std::unique_ptr<T[]> sResampledImage = new(T[resPow2.x * resolution.y]);
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
				wt[i].weight[j] = Lanczos((pos - center) / filterwidth);
			}
		}
		float invSumWts = 1 / (wt[i].weight[0] + wt[i].weight[1] +
			wt[i].weight[2] + wt[i].weight[3]);
		for (int j = 0; j < 4; ++j)
			wt[i].weight[j] *= invSumWts;
		return wt;
	}
}
