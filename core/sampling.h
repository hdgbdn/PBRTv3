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
}

#endif