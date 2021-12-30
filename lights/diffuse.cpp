#include "diffuse.h"
#include "shape.h"

namespace pbrt
{
	DiffuseAreaLight::DiffuseAreaLight(const Transform& LightToWorld, const MediumInterface& mediumInterface, const Spectrum& Lemit, int nSamples, const std::shared_ptr<Shape>& shape)
		:AreaLight(LightToWorld, mediumInterface, nSamples),
	Lemit(Lemit), shape(shape), area(shape->Area())
	{
	}
	Spectrum DiffuseAreaLight::L(const Interaction& intr, const Vector3f& w) const
	{
		return Dot(intr.n, w) > 0 ? Lemit : Spectrum(0.f);
	}
	Spectrum DiffuseAreaLight::Power() const
	{
		return Lemit * area * Pi;
	}
	Spectrum DiffuseAreaLight::Sample_Li(const Interaction& ref, const Point2f& u, Vector3f* wi, float* pdf, VisibilityTester* vis) const
	{
		Interaction pShape = shape->Sample(ref, u);
		pShape.mediumInterface = mediumInterface;
		*wi = Normalize(pShape.p - ref.p);
		*pdf = shape->Pdf(ref, *wi);
		*vis = VisibilityTester(ref, pShape);
		return L(pShape, -*wi);
	}
	float DiffuseAreaLight::Pdf_Li(const Interaction& ref, const Vector3f& wi) const
	{
		return shape->Pdf(ref, wi);
	}
}
