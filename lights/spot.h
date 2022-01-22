#ifndef PBRT_LIGHT_SPOT_H
#define PBRT_LIGHT_SPOT_H

#include "light.h"

namespace pbrt
{
	class SpotLight : public Light
	{
	public:
		SpotLight(const Transform& LightToWorld, const MediumInterface& mediumInterface
			, const Spectrum& I, float totalWidth, float falloffStart);
		Spectrum Sample_Li(const Interaction& ref, const Point2f& u, Vector3f* wi, float* pdf, VisibilityTester* vis) const override;
		float Pdf_Li(const Interaction& ref, const Vector3f& wi) const override;
		float Falloff(const Vector3f& w) const;
		Spectrum Power() const override;
		~SpotLight() = default;
	private:
		const Point3f pLight;
		const Spectrum I;
		const float cosTotalWidth, cosFalloffstart;
	};

    std::shared_ptr<SpotLight> CreateSpotLight(const Transform& lightToWorld, const Medium* medium, const ParamSet& paramSet);
}

#endif