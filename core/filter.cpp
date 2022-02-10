#include "filter.h"

namespace pbrt
{
	Filter::Filter(const Vector2f& radius)
		: radius(radius), invRadius(Vector2f(1 / radius.x, 1 / radius.y))
	{
	}

	GaussianFilter::GaussianFilter(const Vector2f& radius, float alpha)
		: Filter(radius), alpha(alpha),
		  expX(std::exp(-alpha * radius.x * radius.x)), expY(std::exp(-alpha * radius.y * radius.y))
	{
	}

	float GaussianFilter::Evaluate(const Point2f& p) const
	{
		return Gaussian(p.x, expX) * Gaussian(p.y, expY);
	}


}
