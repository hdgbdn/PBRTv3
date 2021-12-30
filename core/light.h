#ifndef PBRT_CORE_LIGHT_H
#define PBRT_CORE_LIGHT_H

#include "medium.h"
#include "scene.h"
#include "transformation.h"

namespace pbrt
{
	enum class LightFlags : int {
		DeltaPosition = 1, DeltaDirection = 2, Area = 4, Infinite = 8
	};
	inline bool IsDeltaLight(int flags)
	{
		return flags & (int)LightFlags::DeltaPosition ||
			flags & (int)LightFlags::DeltaDirection;
	}
	class Light
	{
	public:
		Light(int flags, const Transform& LightToWorld, const MediumInterface& mediumInterface,
			int nSamples = 1);
		virtual Spectrum Power() const = 0;
		virtual void Preprocess(const Scene& scene) { };
		virtual Spectrum Le(const RayDifferential& ray) const;
		virtual Spectrum Sample_Li(const Interaction& ref, const Point2f& u,
			Vector3f* wi, float* pdf,
			VisibilityTester* vis) const = 0;
		virtual float Pdf_Li(const Interaction& ref, const Vector3f& wi) const = 0;
		const int flags;
		const int nSamples;
		const MediumInterface mediumInterface;
	protected:
		const Transform LightToWorld, WorldToLight;
	};
	class VisibilityTester
	{
	public:
		VisibilityTester(const Interaction& p0, const Interaction& p1)
			:p0(p0), p1(p1) {}
		const Interaction& P0() const { return p0; }
		const Interaction& P1() const { return p1; }
		bool Unoccluded(const Scene& scene) const
		{
			return !scene.IntersectP(p0.SpawnRayTo(p1));
		}
		Spectrum Tr(const Scene& scene, Sampler& sampler) const
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
	private:
		Interaction p0, p1;
	};

	class AreaLight : Light
	{
	public:
		AreaLight(const Transform& LightToWorld, const MediumInterface& medium, int nSamples)
			: Light((int)LightFlags::Area, LightToWorld, medium, nSamples) { }
		virtual Spectrum L(const Interaction& intr, const Vector3f& w) const = 0;
	};
}

#endif