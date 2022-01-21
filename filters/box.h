#ifndef PBRT_FILTERS_BOX_H
#define PBRT_FILTERS_BOX_H

#include "filter.h"

namespace pbrt
{


	class BoxFilter : public Filter
	{
	public:
		BoxFilter(const Vector2f& radius);
		float Evaluate(const Point2f& p) const override;
	};

}  // namespace pbrt

#endif  // PBRT_FILTERS_BOX_H