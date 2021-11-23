#include "stratified.h"
#include "sampling.h"

namespace pbrt
{
	StratifiedSampler::StratifiedSampler(int xPixelSamples, int yPixelSamples, bool jitterSamples,
	                                     int nSampledDimensions)
		: PixelSampler(xPixelSamples * yPixelSamples, nSampledDimensions),
		  xPixelSamples(xPixelSamples), yPixelSamples(yPixelSamples),
		  jitterSamples(jitterSamples)
	{
	}

	void StratifiedSampler::StartPixel(const Point2i& p)
	{
		for(size_t i = 0; i < samples1D.size(); ++i)
		{
			StratifiedSample1D(&samples1D[i][0], xPixelSamples * yPixelSamples,
				rng, jitterSamples);
			Shuffle(&samples1D[i][0], xPixelSamples * yPixelSamples, 1, rng);
		}
		for (size_t i = 0; i < samples2D.size(); ++i)
		{
			StratifiedSample2D(&samples2D[i][0], xPixelSamples, yPixelSamples,
			                   rng, jitterSamples);
			Shuffle(&samples2D[i][0], xPixelSamples * yPixelSamples, 1, rng);
		}
		for (size_t i = 0; i < samples1DArraySizes.size(); ++i)
			for (int64_t j = 0; j < samplesPerPixel; ++j)
			{
				int count = samples1DArraySizes[i];
				StratifiedSample1D(&sampleArray1D[i][j * count], count, rng,
				                   jitterSamples);
				Shuffle(&sampleArray1D[i][j * count], count, 1, rng);
			}
		for (size_t i = 0; i < samples2DArraySizes.size(); ++i)
			for (int64_t j = 0; j < samplesPerPixel; ++j)
			{
				int count = samples2DArraySizes[i];
				LatinHypercube(&sampleArray2D[i][j * count].x, count, 2, rng);
			}

		PixelSampler::StartPixel(p);
	}



}