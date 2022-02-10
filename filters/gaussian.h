#ifndef PBRT_FILTERS_GAUSSIAN_H
#define PBRT_FILTERS_GAUSSIAN_H

#include "core/filter.h"

namespace pbrt
{
	class GaussianFilter : public Filter
	{
	public:
		GaussianFilter(const Vector2f& radius, float alpha);
		float Evaluate(const Point2f& p) const override;
	private:
		float Gaussian(float d, float expv) const
		{
			return std::max(float(0), float(std::exp(-alpha * d * d) - expv));
		}
		const float alpha;
		const float expX, expY;
	};

	GaussianFilter* CreateGaussianFilter(const ParamSet& ps);
}

#endif