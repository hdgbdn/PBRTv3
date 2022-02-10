#ifndef PBRT_CORE_FILTER_H
#define PBRT_CORE_FILTER_H

#include "geometry.h"
#include "pbrt.h"

namespace pbrt
{
	class Filter
	{
	public:
		virtual ~Filter() = default;
		Filter(const Vector2f& radius);
		virtual float Evaluate(const Point2f& p) const = 0;
		const Vector2f radius, invRadius;
	};

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
}

#endif