#ifndef PBRT_LIGHT_SPOT_H
#define PBRT_LIGHT_SPOT_H

#include "light.h"

namespace pbrt
{
	class SpotLight : public Light
	{
		SpotLight(const Transform& LightToWorld, const MediumInterface& mediumInterface
			, const Spectrum& I, float totalWidth, float falloffStart);
		Spectrum Sample_Li(const Interaction& ref, const Point2f& u, Vector3f* wi, float* pdf, VisibilityTester* vis) const override;
		float Falloff(const Vector3f& w) const;
		Spectrum Power() const override;
	private:
		const Point3f pLight;
		const Spectrum I;
		const float cosTotalWidth, cosFalloffstart;
	};
}

#endif