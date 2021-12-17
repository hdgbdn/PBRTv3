#include "spot.h"

namespace pbrt
{
	SpotLight::SpotLight(const Transform& LightToWorld, const MediumInterface& mediumInterface, const Spectrum& I,
		float totalWidth, float falloffStart)
			: Light((int)LightFlags::DeltaPosition, LightToWorld, mediumInterface),
			pLight(LightToWorld(Point3f(0, 0, 0))), I(I),
	cosTotalWidth(std::cos(Radians(totalWidth))),
	cosFalloffstart(std::cos(Radians(falloffStart)))
	{
	}
	Spectrum SpotLight::Sample_Li(const Interaction& ref, const Point2f& u, Vector3f* wi, float* pdf, VisibilityTester* vis) const
	{
		*wi = Normalize(pLight - ref.p);
		*pdf = 1.f;
		*vis = VisibilityTester(ref, Interaction(pLight, ref.time, mediumInterface));
		return I * Falloff(-*wi) / DistanceSquared(pLight, ref.p);
	}

	float SpotLight::Falloff(const Vector3f& w) const
	{
		Vector3f wl = Normalize(WorldToLight(w));
		float cosTheta = wl.z;
		if (cosTheta < cosTotalWidth) return 0;
		if (cosTheta > cosFalloffstart) return 1;
		float delta = (cosTheta - cosTotalWidth) / (cosFalloffstart - cosTotalWidth);
		return (delta * delta) * (delta * delta);
	}
	Spectrum SpotLight::Power() const
	{
		return I * 2 * Pi * (1 - .5f * (cosFalloffstart + cosTotalWidth));
	}
}
