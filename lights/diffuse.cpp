#include "diffuse.h"

#include "core/paramset.h"
#include "core/shape.h"

namespace pbrt
{
	DiffuseAreaLight::DiffuseAreaLight(const Transform& LightToWorld, const MediumInterface& mediumInterface, const Spectrum& Lemit, int nSamples, 
		const std::shared_ptr<Shape>& shape, bool twoSided)
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

	std::shared_ptr<AreaLight> CreateDiffuseAreaLight(const Transform& light2world, const Medium* medium,
		const ParamSet& paramSet, const std::shared_ptr<Shape>& shape)
	{
		Spectrum L = paramSet.FindOneSpectrum("L", Spectrum(1.0));
		Spectrum sc = paramSet.FindOneSpectrum("scale", Spectrum(1.0));
		int nSamples = paramSet.FindOneInt("samples",
			paramSet.FindOneInt("nsamples", 1));
		bool twoSided = paramSet.FindOneBool("twosided", false);
		if (PbrtOptions.quickRender) nSamples = std::max(1, nSamples / 4);
		return std::make_shared<DiffuseAreaLight>(light2world, medium, L * sc,
			nSamples, shape, twoSided);
	}
}
