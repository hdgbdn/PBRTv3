#include "sampling.h"

namespace pbrt
{
	void StratifiedSample1D(float* samp, int nSamples, RNG rng, bool jitter)
	{
		float invNSamples = (float)1 / nSamples;
		for (int i = 0; i < nSamples; ++i)
		{
			float delta = jitter ? rng.UniformFloat() : 0.5f;
			samp[i] = std::min((i + delta) * invNSamples, OneMinusEpsilon);
		}
	}

	void StratifiedSample2D(Point2f* samp, int nx, int ny, RNG& rng,
	                        bool jitter)
	{
		float dx = (float)1 / nx, dy = (float)1 / ny;
		for (int y = 0; y < ny; ++y)
			for (int x = 0; x < nx; ++x)
			{
				float jx = jitter ? rng.UniformFloat() : 0.5f;
				float jy = jitter ? rng.UniformFloat() : 0.5f;
				samp->x = std::min((x + jx) * dx, OneMinusEpsilon);
				samp->y = std::min((y + jy) * dy, OneMinusEpsilon);
				++samp;
			}
	}

	void LatinHypercube(float* samples, int nSamples, int nDim, RNG& rng)
	{
		float invNSamples = (float)1 / nSamples;
		for (int i = 0; i < nSamples; ++i)
			for (int j = 0; j < nDim; ++j)
			{
				float sj = (i + (rng.UniformFloat())) * invNSamples;
				samples[nDim * i + j] = std::min(sj, OneMinusEpsilon);
			}
		for (int i = 0; i < nDim; ++i) {
			for (int j = 0; j < nSamples; ++j) {
				int other = j + rng.UniformUInt32(nSamples - j);
				std::swap(samples[nDim * j + i], samples[nDim * other + i]);
			}
		}
	}

}
