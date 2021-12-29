#ifndef PBRT_CORE_INTERACTION_H
#define PBRT_CORE_INTERACTION_H

#include "reflection.h"
#include "medium.h"

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
		Ray SpawnRay(const Vector3f& d) const {
			Point3f o = OffsetRayOrigin(p, pError, n, d);
			return Ray(o, d, Infinity, time, GetMedium(d));
		}
		Ray SpawnRayTo(const Point3f& p2) const {
			Point3f origin = OffsetRayOrigin(p, pError, n, p2 - p);
			Vector3f d = p2 - origin;
			return Ray(origin, d, 1 - ShadowEpsilon, time, GetMedium(d));
		}
		Ray SpawnRayTo(const Interaction& it) const {
			Point3f origin = OffsetRayOrigin(p, pError, n, it.p - p);
			Point3f target = OffsetRayOrigin(it.p, it.pError, it.n, origin - it.p);
			Vector3f d = target - origin;
			return Ray(origin, d, 1 - ShadowEpsilon, time, GetMedium(d));
		}
		const Medium* GetMedium() const {
			assert(mediumInterface.inside == mediumInterface.outside);
			return mediumInterface.inside;
		}
		const Medium* GetMedium(const Vector3f& w) const {
			return Dot(w, n) > 0 ? mediumInterface.outside :
				mediumInterface.inside;
		}
		Interaction(const Point3f& p, const Vector3f& wo, float time,
			const MediumInterface& mediumInterface)
			: p(p), time(time), wo(wo), mediumInterface(mediumInterface) {}
		Interaction(const Point3f& p, float time,
			const MediumInterface& mediumInterface)
			: p(p), time(time), mediumInterface(mediumInterface) {}
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
		void ComputeDifferentials(const RayDifferential& ray) const;
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
		mutable Vector3f dpdx, dpdy;
		mutable float dudx = 0, dvdx = 0, dudy = 0, dvdy = 0;
	};
}

#endif