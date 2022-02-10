#ifndef PBRT_FILTERS_BOX_H
#define PBRT_FILTERS_BOX_H

#include "core/filter.h"

namespace pbrt
{
	class BoxFilter : public Filter
	{
	public:
		BoxFilter(const Vector2f& radius) : Filter(radius) {}
		float Evaluate(const Point2f& p) const override;
	};

	BoxFilter* CreateBoxFilter(const ParamSet& ps);

}  // namespace pbrt

#endif  // PBRT_FILTERS_BOX_H