#include "point.h"

namespace pbrt
{
	PointLight::PointLight(const Transform& LightToWorld, const MediumInterface& mediumInterface, const Spectrum& I)
		:Light((int)LightFlags::DeltaPosition, LightToWorld, mediumInterface),
		pLight(LightToWorld(Point3f(0, 0, 0))), I(I) { }

	Spectrum pbrt::PointLight::Power() const
	{
		return 4 * Pi * I;
	}
	Spectrum PointLight::Sample_Li(const Interaction& ref, const Point2f& u, Vector3f* wi, float* pdf, VisibilityTester* vis) const
	{
		*wi = Normalize(pLight - ref.p);
		*pdf = 1.f;
		*vis = VisibilityTester(ref, Interaction(pLight, ref.time, mediumInterface));
		return I / DistanceSquared(pLight, ref.p);
	}
	float PointLight::Pdf_Li(const Interaction& ref, const Vector3f& wi) const
	{
		return 0.0f;
	}
}



