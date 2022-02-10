#include "triangle.h"
#include "core/paramset.h"

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

	TriangleFilter* CreateTriangleFilter(const ParamSet& ps)
	{
		float xw = ps.FindOneFloat("xwidth", 0.5f);
		float yw = ps.FindOneFloat("ywidth", 0.5f);
		return new TriangleFilter(Vector2f(xw, yw));
	}
}
