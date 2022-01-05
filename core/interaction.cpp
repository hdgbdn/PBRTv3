#include "interaction.h"
#include "light.h"
#include "primitive.h"
#include "shape.h"
#include "transformation.h"

namespace pbrt
{
	Interaction::Interaction(): time(0)
	{
	}

	Interaction::Interaction(const Point3f& p, const Normal3f& n, const Vector3f& pError, const Vector3f& wo,
	                         float time, const MediumInterface& mediumInterface):
		p(p), time(0), pError(pError), wo(wo), n(n),
		mediumInterface(mediumInterface)
	{
	}

	bool Interaction::IsSurfaceInteraction() const
	{
		return n != Normal3f();
	}

	Ray Interaction::SpawnRay(const Vector3f& d) const
	{
		Point3f o = OffsetRayOrigin(p, pError, n, d);
		return Ray(o, d, Infinity, time, GetMedium(d));
	}

	Ray Interaction::SpawnRayTo(const Point3f& p2) const
	{
		Point3f origin = OffsetRayOrigin(p, pError, n, p2 - p);
		Vector3f d = p2 - origin;
		return Ray(origin, d, 1 - ShadowEpsilon, time, GetMedium(d));
	}

	Ray Interaction::SpawnRayTo(const Interaction& it) const
	{
		Point3f origin = OffsetRayOrigin(p, pError, n, it.p - p);
		Point3f target = OffsetRayOrigin(it.p, it.pError, it.n, origin - it.p);
		Vector3f d = target - origin;
		return Ray(origin, d, 1 - ShadowEpsilon, time, GetMedium(d));
	}

	const Medium* Interaction::GetMedium() const
	{
		assert(mediumInterface.inside == mediumInterface.outside);
		return mediumInterface.inside;
	}

	const Medium* Interaction::GetMedium(const Vector3f& w) const
	{
		return Dot(w, n) > 0 ? mediumInterface.outside : mediumInterface.inside;
	}

	Interaction::Interaction(const Point3f& p, const Vector3f& wo, float time,
	                         const MediumInterface& mediumInterface): p(p), time(time), wo(wo),
	                                                                  mediumInterface(mediumInterface)
	{
	}

	Interaction::Interaction(const Point3f& p, float time, const MediumInterface& mediumInterface): p(p), time(time),
		mediumInterface(mediumInterface)
	{
	}

	SurfaceInteraction::SurfaceInteraction() = default;

	SurfaceInteraction::SurfaceInteraction(const Point3f& p, const Vector3f& pError,
	                                       const Point2f& uv, const Vector3f& wo,
	                                       const Vector3f& dpdu, const Vector3f& dpdv,
	                                       const Normal3f& dndu, const Normal3f& dndv,
	                                       float time, const Shape* shape):
		Interaction(p, Normal3f(Normalize(Cross(dpdu, dpdv))),
		            pError, wo, time, nullptr), uv(uv),
		dpdu(dpdu),
		dpdv(dpdv),
		dndu(dndu),
		dndv(dndv),
		shape(shape)
	{
		shading.n = n;
		shading.dpdu = dpdu;
		shading.dpdv = dpdv;
		shading.dndu = dndu;
		shading.dndv = dndv;
		if (shape != nullptr && (shape->reverseOrientation ^
			shape->transformSwapsHandedness))
		{
			n *= -1;
			shading.n *= -1;
		}
	}

	void SurfaceInteraction::SetShadingGeometry(const Vector3f& dpdus, const Vector3f& dpdvs, const Normal3f& dndus,
	                                            const Normal3f& dndvs, bool orientationIsAuthoritative)
	{
		shading.n = Normalize(static_cast<Normal3f>(Cross(dpdus, dpdvs)));
		if (nullptr != shape && (shape->reverseOrientation ^
			shape->transformSwapsHandedness))
			shading.n = -shading.n;
		if (orientationIsAuthoritative)
			n = Faceforward(n, shading.n);
		else
			shading.n = Faceforward(shading.n, n);
		shading.dpdu = dpdu;
		shading.dpdv = dpdv;
		shading.dndu = dndu;
		shading.dndv = dndv;
	}

	void SurfaceInteraction::ComputeScatteringFunctions(const RayDifferential& ray, MemoryArena& arena,
	                                                    bool allowMultipleLobes, TransportMode mode)
	{
		ComputeDifferentials(ray);
		primitive->ComputeScatteringFunctions(this, arena, mode,
		                                      allowMultipleLobes);
	}

	void SurfaceInteraction::ComputeDifferentials(const RayDifferential& ray) const
	{
		if (ray.hasDifferentials)
		{
			float d = -Dot(n, Vector3f(p.x, p.y, p.z));
			float tx = (-Dot(n, Vector3f(ray.rxOrigin)) - d) /
				Dot(n, ray.rxDirection);
			Point3f px = ray.rxOrigin + tx * ray.rxDirection;
			float ty = (-Dot(n, Vector3f(ray.ryOrigin)) - d) /
				Dot(n, ray.ryDirection);
			Point3f py = ray.ryOrigin + ty * ray.ryDirection;
			dpdx = px - p;
			dpdy = py - p;
			int dim[2];
			if (std::abs(n.x) > std::abs(n.y) && std::abs(n.x) > std::abs(n.z))
			{
				dim[0] = 1;
				dim[1] = 2;
			}
			else if (std::abs(n.y) > std::abs(n.z))
			{
				dim[0] = 0;
				dim[1] = 2;
			}
			else
			{
				dim[0] = 0;
				dim[1] = 1;
			}
			float A[2][2] = {
				{dpdu[dim[0]], dpdv[dim[0]]},
				{dpdu[dim[1]], dpdv[dim[1]]}
			};
			float Bx[2] = {px[dim[0]] - p[dim[0]], px[dim[1]] - p[dim[1]]};
			float By[2] = {py[dim[0]] - p[dim[0]], py[dim[1]] - p[dim[1]]};
			if (!SolveLinearSystem2x2(A, Bx, &dudx, &dvdx))
				dudx = dvdx = 0;
			if (!SolveLinearSystem2x2(A, By, &dudy, &dvdy))
				dudy = dvdy = 0;
		}
		else
		{
			dudx = dvdx = 0;
			dudy = dvdy = 0;
			dpdx = dpdy = Vector3f(0, 0, 0);
		}
	}

	Spectrum SurfaceInteraction::Le(const Vector3f& w) const
	{
		const AreaLight* area = primitive->GetAreaLight();
		return area ? area->L(*this, w) : Spectrum(0.f);
	}
}
