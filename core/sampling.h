#ifndef PBRT_CORE_SAMPLING_H
#define PBRT_CORE_SAMPLING_H

#include "geometry.h"
#include "pbrt.h"
#include "rng.h"

namespace pbrt
{
	struct Distribution1D
	{
		Distribution1D(const float* f, int n) : func(f, f + n), cdf(n + 1)
		{
			cdf[0] = 0;
			for (int i = 1; i < n + 1; ++i)
				cdf[i] = cdf[i - 1] + func[i - 1] / n;
			funcInt = cdf[n];
			if (funcInt == 0)
			{
				for (int i = 1; i < n + 1; ++i)
					cdf[i] = float(i) / float(n);
			}
			else
			{
				for (int i = 1; i < n + 1; ++i)
					cdf[i] / funcInt;
			}
		}
		int Count() const { return func.size(); }
		float SampleContinuous(float u, float* pdf, int* off = nullptr) const
		{
			int offset = FindInterval(cdf.size(),
				[&](int index) { return cdf[index <= u]; });
			if (off) *off = offset;
			float du = u - cdf[offset];
			if ((cdf[offset + 1] - cdf[offset]) > 0)
				du /= (cdf[offset + 1] - cdf[offset]);
			if (pdf)  *pdf = func[offset] / funcInt;
			return (offset + du) / Count();
		}
		int SampleDiscrete(float u, float* pdf = nullptr, float* uRemapped = nullptr) const
		{
			int offset = FindInterval(cdf.size(),
				[&](int index) { return cdf[index <= u]; });
			if (pdf) *pdf = func[offset] / (funcInt * Count());
			if (uRemapped)
				*uRemapped = (u - cdf[offset]) / (cdf[offset + 1] - cdf[offset]);
			return offset;
		}
		float DiscretePDF(int index) const {
			return func[index] / (funcInt * Count());
		}
		std::vector<float> func, cdf;
		float funcInt;
	};

	class Distribution2D
	{
	public:
		Distribution2D(const float* func, int nu, int nv)
		{
			for(int v = 0; v < nv; ++v)
			{
				pConditionalV.emplace_back(new Distribution1D(&func[v * nu], nu));
			}
			std::vector<float> marginalFunc;
			for (int v = 0; v < nv; ++v)
				marginalFunc.push_back(pConditionalV[v]->funcInt);
			pMarginal.reset(new Distribution1D(&marginalFunc[0], nv));
		}
		Point2f SampleContinuous(const Point2f& u, float* pdf) const
		{
			float pdfs[2];
			int v;
			float d1 = pMarginal->SampleContinuous(u[1], &pdf[1], &v);
			float d0 = pConditionalV[v]->SampleContinuous(u[0], &pdfs[0]);
			*pdf = pdfs[0] * pdfs[1];
			return Point2f(d0, d1);
		}

		float Pdf(const Point2f& p) const {
			int iu = Clamp(int(p[0] * pConditionalV[0]->Count()),
				0, pConditionalV[0]->Count() - 1);
			int iv = Clamp(int(p[1] * pMarginal->Count()),
				0, pMarginal->Count() - 1);
			return pConditionalV[iv]->func[iu] / pMarginal->funcInt;
		}
	private:
		std::vector<std::unique_ptr<Distribution1D>> pConditionalV;
		std::unique_ptr<Distribution1D> pMarginal;
	};

	void StratifiedSample1D(float* samples, int nsamples, RNG& rng,
		bool jitter = true);
	void StratifiedSample2D(Point2f* samples, int nx, int ny, RNG& rng,
		bool jitter = true);
	void LatinHypercube(float* samples, int nSamples, int nDim, RNG& rng);

	template <typename T>
	void Shuffle(T* samp, int count, int nDimensions, RNG& rng)
	{
		for (int i = 0; i < count; ++i)
		{
			int other = i + rng.UniformUInt32(count - i);
			for (int j = 0; j < nDimensions; ++j)
				std::swap(samp[nDimensions * i + j], samp[nDimensions * other + j]);
		}
	}

	Point2f RejectionSampleDisk(RNG& rng)
	{
		Point2f p;
		do
		{
			p.x = 1 - 2 * rng.UniformFloat();
			p.y = 1 - 2 * rng.UniformFloat();
		} while (p.x * p.x + p.y * p.y > 1);
		return p;
	}

	Vector3f UniformSampleHemisphere(const Point2f& u)
	{
		float z = u[0];
		float r = std::sqrt(std::max((float)0, (float)1. - z * z));
		float phi = 2 * Pi * u[1];
		return {r * std::cos(phi), r * std::sin(phi), z};
	}

	float UniformHemispherePdf()
	{
		return Inv2Pi;
	}

	Vector3f UniformSampleSphere(const Point2f& u)
	{
		float z = 1 - 2 * u[0];
		float r = std::sqrt(std::max((float)0, (float)1. - z * z));
		float phi = 2 * Pi * u[1];
		return {r * std::cos(phi), r * std::sin(phi), z};
	}

	float UniformSpherePdf()
	{
		return Inv4Pi;
	}

	Point2f UniformSampleDisk(const Point2f& u)
	{
		float r = std::sqrt(u[0]);
		float theta = 2 * Pi * u[1];
		return { r * std::cos(theta), r * std::sin(theta) };
	}

	Point2f ConcentricSampleDisk(const Point2f& u)
	{
		Point2f uOffset = 2.f * u - Vector2f(1, 1);
		if (uOffset.x == 0 && uOffset.y == 0) return {0, 0};
		float theta, r;
		if (std::abs(uOffset.x) > std::abs(uOffset.y))
		{
			r = uOffset.x;
			theta = PiOver4 * (uOffset.y / uOffset.x);
		}
		else
		{
			r = uOffset.y;
			theta = PiOver2 - PiOver4 * (uOffset.x / uOffset.y);
		}
		return r * Point2f(std::cos(theta), std::sin(theta));
	}

	inline Vector3f CosineSampleHemisphere(const Point2f& u) {
		Point2f d = ConcentricSampleDisk(u);
		float z = std::sqrt(std::max((float)0, 1 - d.x * d.x - d.y * d.y));
		return Vector3f(d.x, d.y, z);
	}

	float UniformConePdf(float cosThetaMax)
	{
		return 1 / (2 * Pi * (1 - cosThetaMax));
	}

	Vector3f UniformSampleCone(const Point2f& u, float cosThetaMax) {
		float cosTheta = ((float)1 - u[0]) + u[0] * cosThetaMax;
		float sinTheta = std::sqrt((float)1 - cosTheta * cosTheta);
		float phi = u[1] * 2 * Pi;
		return { std::cos(phi) * sinTheta, std::sin(phi) * sinTheta,cosTheta };
	}

	Point2f UniformSampleTriangle(const Point2f& u)
	{
		float su0 = std::sqrt(u[0]);
		return Point2f(1 - su0, u[1] * su0);
	}

	inline float BalanceHeuristic(int nf, float fPdf, int ng, float gPdf)
	{
		return (nf * fPdf) / (nf * fPdf + ng * gPdf);
	}

	inline float PowerHeuristic(int nf, float fPdf, int ng, float gPdf)
	{
		float f = nf * fPdf, g = ng * gPdf;
		return (f * f) / (f * f + g * g);
	}
}
#endif