#include "spot.h"
#include "paramset.h"

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

	float SpotLight::Pdf_Li(const Interaction& ref, const Vector3f& wi) const
	{
		return 0.f;
	}

	std::shared_ptr<SpotLight> CreateSpotLight(const Transform& lightToWorld, const Medium* medium,
	                                           const ParamSet& paramSet)
	{
		Spectrum I = paramSet.FindOneSpectrum("I", Spectrum(1.0));
		Spectrum sc = paramSet.FindOneSpectrum("scale", Spectrum(1.0));
		float coneangle = paramSet.FindOneFloat("coneangle", 30.);
		float conedelta = paramSet.FindOneFloat("conedeltaangle", 5.);
		// Compute spotlight world to light transformation
		Point3f from = paramSet.FindOnePoint3f("from", Point3f(0, 0, 0));
		Point3f to = paramSet.FindOnePoint3f("to", Point3f(0, 0, 1));
		Vector3f dir = Normalize(to - from);
		Vector3f du, dv;
		CoordinateSystem(dir, &du, &dv);
		Transform dirToZ =
			Transform(Matrix4x4(du.x, du.y, du.z, 0., dv.x, dv.y, dv.z, 0., dir.x,
				dir.y, dir.z, 0., 0, 0, 0, 1.));
		Transform light2world =
			lightToWorld * Translate(Vector3f(from.x, from.y, from.z)) * Inverse(dirToZ);
		return std::make_shared<SpotLight>(light2world, medium, I * sc, coneangle,
			coneangle - conedelta);
	}
}
