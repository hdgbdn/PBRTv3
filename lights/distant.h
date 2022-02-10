#ifndef PBRT_LIGHTS_DISTANT_H
#define PBRT_LIGHTS_DISTANT_H

#include "core/light.h"

namespace pbrt
{
	class DistantLight : public Light
	{
	public:
		DistantLight(const Transform& LightToWorld, const Spectrum& L, const Vector3f& wLight);
		void Preprocess(const Scene& scene) override;
		Spectrum Sample_Li(const Interaction& ref, const Point2f& u, Vector3f* wi, float* pdf, VisibilityTester* vis) const override;
		Spectrum Power() const override;
	private:
		const Spectrum L;
		const Vector3f wLight;
		Point3f worldCenter;
		float worldRadius;
	};
}

#endif