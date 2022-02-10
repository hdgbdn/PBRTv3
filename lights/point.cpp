#include "point.h"
#include "core/paramset.h"

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

    std::shared_ptr<PointLight> CreatePointLight(const Transform &lightToWorld, const Medium *medium, const ParamSet &paramSet)
    {
        Spectrum I = paramSet.FindOneSpectrum("I", Spectrum(1.0));
        Spectrum sc = paramSet.FindOneSpectrum("scale", Spectrum(1.0));
        Point3f P = paramSet.FindOnePoint3f("from", Point3f(0, 0, 0));
        Transform l2w = Translate(Vector3f(P.x, P.y, P.z)) * lightToWorld;
        return std::make_shared<PointLight>(l2w, medium, I * sc);
    }
}



