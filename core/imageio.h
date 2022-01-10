#ifndef PBRT_CORE_IMAGEIO_H
#define PBRT_CORE_IMAGEIO_H

#include "pbrt.h"
#include "geometry.h"
#include "spectrum.h"

namespace pbrt
{
	std::unique_ptr<RGBSpectrum[]> ReadImage(const std::string& name,
		Point2i* resolution);
	void WriteImage(const std::string& name, const float* rgb, const Bounds2i& outputBounds, const Point2i& totalResolution);
}

#endif