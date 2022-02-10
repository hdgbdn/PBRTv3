#include "triangle.h"

namespace pbrt
{

	TriangleFilter::TriangleFilter(const Vector2f& radius)
		: Filter(radius)
	{
	}

	float TriangleFilter::Evaluate(const Point2f& p) const
	{
		return std::max((float)0, radius.x - std::abs(p.x)) *
			std::max((float)0, radius.y - std::abs(p.y));
	}
}
