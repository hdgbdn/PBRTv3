#ifndef PBRT_SAMPLERS_STRATIFIED_H
#define PBRT_SAMPLERS_STRATIFIED_H

#include "sampler.h"

namespace pbrt
{
	class StratifiedSampler : public PixelSampler
	{
	public:
		StratifiedSampler(int xPixelSamples, int yPixelSamples,
		                  bool jitterSamples, int nSampledDimensions);
		void StartPixel(const Point2i& p) override;
	private:
		const int xPixelSamples, yPixelSamples;
		const bool jitterSamples;
	};
}

#endif