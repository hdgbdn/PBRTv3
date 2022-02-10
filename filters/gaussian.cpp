#include "gaussian.h"
#include "core/paramset.h"

namespace pbrt
{
	GaussianFilter::GaussianFilter(const Vector2f& radius, float alpha)
		: Filter(radius), alpha(alpha),
		expX(std::exp(-alpha * radius.x * radius.x)), expY(std::exp(-alpha * radius.y * radius.y))
	{
	}

	float GaussianFilter::Evaluate(const Point2f& p) const
	{
		return Gaussian(p.x, expX) * Gaussian(p.y, expY);
	}

	GaussianFilter* CreateGaussianFilter(const ParamSet& ps)
	{
		float xw = ps.FindOneFloat("xwidth", 0.5f);
		float yw = ps.FindOneFloat("ywidth", 0.5f);
		float alpha = ps.FindOneFloat("alpha", 2.f);
		return new GaussianFilter(Vector2f(xw, yw), alpha);
	}
}
