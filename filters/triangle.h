#ifndef PBRT_FILTERS_TRIANGLE_H
#define PBRT_FILTERS_TRIANGLE_H

#include "core/filter.h"

namespace pbrt
{
	class TriangleFilter : public Filter
	{
	public:
		TriangleFilter(const Vector2f& radius);
		float Evaluate(const Point2f& p) const override;
	};

	TriangleFilter* CreateTriangleFilter(const ParamSet& ps);
}

#endif
