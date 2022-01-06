#ifndef PBRT_CORE_LIGHT_H
#define PBRT_CORE_LIGHT_H

#include "medium.h"
#include "transformation.h"
#include "interaction.h"

namespace pbrt
{
	enum class LightFlags : int {
		DeltaPosition = 1, DeltaDirection = 2, Area = 4, Infinite = 8
	};
	inline bool IsDeltaLight(int flags);

	class Light
	{
	public:
		Light(int flags, const Transform& LightToWorld, const MediumInterface& mediumInterface,
			int nSamples = 1);
		virtual Spectrum Power() const = 0;
		virtual void Preprocess(const Scene& scene);;
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
		VisibilityTester();
		VisibilityTester(const Interaction& p0, const Interaction& p1);
		const Interaction& P0() const;
		const Interaction& P1() const;

		bool Unoccluded(const Scene& scene) const;

		Spectrum Tr(const Scene& scene, Sampler& sampler) const;
	private:
		Interaction p0, p1;
	};

	class AreaLight : public Light
	{
	public:
		AreaLight(const Transform& LightToWorld, const MediumInterface& medium, int nSamples);
		virtual Spectrum L(const Interaction& intr, const Vector3f& w) const = 0;
	};
}

#endif