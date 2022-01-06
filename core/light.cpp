#include "light.h"
#include "scene.h"

namespace pbrt
{
	bool IsDeltaLight(int flags)
	{
		return flags & (int)LightFlags::DeltaPosition ||
			flags & (int)LightFlags::DeltaDirection;
	}

	void Light::Preprocess(const Scene& scene)
	{ }

	VisibilityTester::VisibilityTester() = default;

	VisibilityTester::VisibilityTester(const Interaction& p0, const Interaction& p1):p0(p0), p1(p1)
	{}

	const Interaction& VisibilityTester::P0() const
	{ return p0; }

	const Interaction& VisibilityTester::P1() const
	{ return p1; }

	bool VisibilityTester::Unoccluded(const Scene& scene) const
	{
		return !scene.IntersectP(p0.SpawnRayTo(p1));
	}

	Spectrum VisibilityTester::Tr(const Scene& scene, Sampler& sampler) const
	{
		Ray ray(p0.SpawnRayTo(p1));
		Spectrum Tr(1.f);
		while(true)
		{
			SurfaceInteraction isect;
			bool hitSurface = scene.Intersect(ray, &isect);
			if (hitSurface && isect.primitive->GetMaterial() != nullptr)
				return Spectrum(0.f);
			if (ray.medium)
				Tr *= ray.medium->Tr(ray, sampler);
			if (!hitSurface)
				break;
			ray = isect.SpawnRayTo(p1);
		}
		return Tr;
	}

	AreaLight::AreaLight(const Transform& LightToWorld, const MediumInterface& medium, int nSamples): Light((int)LightFlags::Area, LightToWorld, medium, nSamples)
	{ }

	Light::Light(int flags, const Transform& LightToWorld, const MediumInterface& mediumInterface, int nSamples)
		: flags(flags), nSamples(std::max(1, nSamples)),
	mediumInterface(mediumInterface), LightToWorld(LightToWorld), WorldToLight(Inverse(LightToWorld))
	{
	}
	Spectrum Light::Le(const RayDifferential& ray) const
	{
		return Spectrum(0.f);
	}
}
