#include "mitchell.h"

namespace pbrt
{
	float MitchellFilter::Evaluate(const Point2f& p) const
	{
		return Mitchell1D(p.x * invRadius.x) * Mitchell1D(p.y * invRadius.y);
	}

}
