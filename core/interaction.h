#ifndef PBRT_CORE_INTERACTION_H
#define PBRT_CORE_INTERACTION_H

#include "geometry.h"
#include "material.h"
#include "reflection.h"
#include "medium.h"
#include "shape.h"

namespace pbrt
{
	struct Interaction
	{
		Interaction() : time(0) {}
		Interaction(const Point3f& p, const Normal3f& n,
			const Vector3f& pError, const Vector3f& wo, float time,
			const MediumInterface& mediumInterface) :
			p(p), time(0), pError(pError), wo(wo), n(n),
			mediumInterface(mediumInterface) {}
		bool IsSurfaceInteraction() const
		{
			return n != Normal3f();
		}
		Point3f p;
		float time;
		Vector3f pError;
		Vector3f wo;
		Normal3f n;
		MediumInterface mediumInterface;
	};
	class SurfaceInteraction : public Interaction
	{
	public:
		SurfaceInteraction() = default;
		SurfaceInteraction(const Point3f& p, const Vector3f& pError,
			const Point2f& uv, const Vector3f& wo,
			const Vector3f& dpdu, const Vector3f& dpdv,
			const Normal3f& dndu, const Normal3f& dndv,
			float time, const Shape* shape);
		void SetShadingGeometry(const Vector3f& dpdus, 
			const Vector3f& dpdvs, const Normal3f& dndus,
			const Normal3f& dndvs, bool orientationIsAuthoritative);
		void ComputeScatteringFunctions(
			const RayDifferential& ray, MemoryArena& arena,
			bool allowMultipleLobes = false,
			TransportMode mode = TransportMode::Radiance);
		Spectrum Le(const Vector3f& w) const;
		Point2f uv;
		Vector3f dpdu, dpdv;
		Normal3f dndu, dndv;
		const Shape* shape;
		struct {
			Normal3f n;
			Vector3f dpdu, dpdv;
			Normal3f dndu, dndv;
		} shading;
		BSDF* bsdf = nullptr;
		BSSRDF* bssrdf = nullptr;
		const Primitive* primitive = nullptr;
	};
}

#endif