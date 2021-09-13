#ifndef PBRT_CORE_SAMPLER_H
#define PBRT_CORE_SAMPLER_H

#include "pbrt.h"

namespace pbrt
{
	class Sampler
	{
	public:
		virtual std::unique_ptr<Sampler> Clone(int seed) = 0;
		virtual void StartPixel(const Point2i& p);
		virtual bool StartNextSample();
		CameraSample GetCameraSample(const Point2i& pRaster);
		const int64_t samplesPerPixel;
	};
}

#endif