#ifndef PBRT_CORE_PARALLEL_H
#define PBRT_CORE_PARALLEL_H

#include "pbrt.h"
#include "geometry.h"
#include <functional>

namespace pbrt
{
	void ParallelFor2D(std::function<void(Point2i)> func, const Point2i &count);
	void ParallelFor(std::function<void(int64_t)> func, int64_t count,
		int chunkSize = 1);
}



#endif