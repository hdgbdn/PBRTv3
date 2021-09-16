#ifndef PBRT_CORE_LIGHT_H
#define PBRT_CORE_LIGHT_H

#include "pbrt.h"

namespace pbrt
{
	class Light
	{
	public:
		void Preprocess(const Scene& scene);
		virtual Spectrum Le(const RayDifferential& ray) const;
		virtual Spectrum Sample_Li(const Interaction& ref, const Point2f& u,
			Vector3f* wi, float* pdf,
			VisibilityTester* vis) const = 0;
		virtual Spectrum Sample_Le(const Point2f& u1, const Point2f& u2, float time,
			Ray* ray, Normal3f* nLight, float* pdfPos,
			float* pdfDir) const = 0;
	};
	class VisibilityTester
	{
	public:
		bool Unoccluded(const Scene& scene) const;
	};
}

#endif