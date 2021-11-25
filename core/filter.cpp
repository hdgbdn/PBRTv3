#include "filter.h"

namespace pbrt
{
	Filter::Filter(const Vector2f& radius)
		: radius(radius), invRadius(Vector2f(1 / radius.x, 1 / radius.y))
	{
	}

	BoxFilter::BoxFilter(const Vector2f& radius)
		: Filter(radius)
	{
	}

	float BoxFilter::Evaluate(const Point2f& p) const
	{
		return 1.f;
	}

	TriangleFilter::TriangleFilter(const Vector2f& radius)
		: Filter(radius)
	{
	}

	float TriangleFilter::Evaluate(const Point2f& p) const
	{
		return std::max((float)0, radius.x - std::abs(p.x)) *
			std::max((float)0, radius.y - std::abs(p.y));
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
