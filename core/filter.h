#ifndef PBRT_CORE_FILTER_H
#define PBRT_CORE_FILTER_H

#include "geometry.h"
#include "pbrt.h"

namespace pbrt
{
	class Filter
	{
	public:
		virtual ~Filter() = default;
		Filter(const Vector2f& radius);
		virtual float Evaluate(const Point2f& p) const = 0;
		const Vector2f radius, invRadius;
	};
}

#endif