#ifndef PBRT_LIGHT_POINT_H
#define PBRT_LIGHT_POINT_H

#include "core/light.h"

namespace pbrt
{
	class PointLight : public Light
	{
	public:
		PointLight(const Transform& LightToWorld, const MediumInterface& mediumInterface, const Spectrum& I);
		Spectrum Power() const override;
		Spectrum Sample_Li(const Interaction& ref, const Point2f& u, Vector3f* wi, float* pdf,
			VisibilityTester* vis) const override;
		float Pdf_Li(const Interaction& ref, const Vector3f& wi) const override;
	private:
		const Point3f pLight;
		const Spectrum I;
	};

    std::shared_ptr<PointLight> CreatePointLight(const Transform& l2w, const Medium* medium, const ParamSet& param);
}

#endif