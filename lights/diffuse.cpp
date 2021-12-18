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
}
