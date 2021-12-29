#include "cylinder.h"
#include "efloat.h"

namespace pbrt
{
	Cylinder::Cylinder(const std::shared_ptr<Transform>& ObjectToWorld, const std::shared_ptr<Transform>& WorldToObject, bool reverseOrientation, float radius, float zMin, float zMax, float phiMax)
		: Shape(ObjectToWorld, WorldToObject, reverseOrientation),
		radius(radius), zMin(std::min(zMin, zMax)),
		zMax(std::max(zMin, zMax)),
		phiMax(Radians(Clamp(phiMax, 0, 360))) {}

	Bounds3f Cylinder::ObjectBound() const
	{
		return Bounds3f(Point3f(-radius, -radius, zMin),
			Point3f(radius, radius, zMax));
	}

	bool Cylinder::Intersect(const Ray& r, float* tHit, SurfaceInteraction* isect, bool testAlphaTexture) const
	{
		Vector3f oErr, dErr;
		Ray ray = (*WorldToObject)(r, &oErr, &dErr);
		EFloat ox(ray.o.x, oErr.x), oy(ray.o.y, oErr.y), oz(ray.o.z, oErr.z);
		EFloat dx(ray.d.x, dErr.x), dy(ray.d.y, dErr.y), dz(ray.d.z, dErr.z);

		EFloat a = dx * dx + dy * dy;
		EFloat b = 2 * (dx * ox + dy * oy);
		EFloat c = ox * ox + oy * oy - static_cast<EFloat>(radius) * static_cast<EFloat>(radius);
		EFloat t0, t1;
		if (!Quadratic(a, b, c, &t0, &t1))
			return false;
		if (t0.UpperBound() > ray.tMax || t1.LowerBound() <= 0)
			return false;
		EFloat tShapeHit = t0;
		if (tShapeHit.LowerBound() <= 0)
		{
			tShapeHit = t1;
			if (tShapeHit.UpperBound() > ray.tMax)
				return false;
		}
		Point3f pHit = ray(static_cast<float>(tShapeHit));
		// TODO Refine cylinder intersection point
		float phi = std::atan2(pHit.y, pHit.x);
		if (phi < 0) phi += 2 * Pi;
		if(pHit.z < zMin || pHit.z > zMax || phi > phiMax)
		{
			if (tShapeHit == t1) return false;
			if (t1.UpperBound() > ray.tMax) return false;
			pHit = ray(static_cast<float>(tShapeHit));
			// TODO Refine cylinder intersection point
			phi = std::atan2(pHit.y, pHit.x);
			if (phi < 0) phi += 2 * Pi;
			if (pHit.z < zMin || pHit.z > zMax || phi > phiMax) return false;
		}

		float u = phi / phiMax;
		float v = (pHit.z - zMin) / (zMax - zMin);
		Vector3f dpdu(-phiMax * pHit.y, phiMax * pHit.x, 0);
		Vector3f dpdv(0, 0, zMax - zMin);

		// TODO Compute $\dndu$ and $\dndv$ from fundamental form coefficients
		Normal3f dndu;
		Normal3f dndv;
		Vector3f pError;
		*isect = (*ObjectToWorld)(SurfaceInteraction(
			pHit, pError, Point2f(u, v), -ray.d, dpdu, dpdv, dndu, dndv, r.time, this));
		*tHit = static_cast<float>(tShapeHit);
		return true;
	}

	bool Cylinder::IntersectP(const Ray& r, bool testAlphaTexture) const
	{
		Vector3f oErr, dErr;
		Ray ray = (*WorldToObject)(r, &oErr, &dErr);
		EFloat ox(ray.o.x, oErr.x), oy(ray.o.y, oErr.y), oz(ray.o.z, oErr.z);
		EFloat dx(ray.d.x, dErr.x), dy(ray.d.y, dErr.y), dz(ray.d.z, dErr.z);

		EFloat a = dx * dx + dy * dy;
		EFloat b = 2 * (dx * ox + dy * oy);
		EFloat c = ox * ox + oy * oy - static_cast<EFloat>(radius) * static_cast<EFloat>(radius);
		EFloat t0, t1;
		if (!Quadratic(a, b, c, &t0, &t1))
			return false;
		if (t0.UpperBound() > ray.tMax || t1.LowerBound() <= 0)
			return false;
		EFloat tShapeHit = t0;
		if (tShapeHit.LowerBound() <= 0)
		{
			tShapeHit = t1;
			if (tShapeHit.UpperBound() > ray.tMax)
				return false;
		}
		Point3f pHit = ray(static_cast<float>(tShapeHit));
		// TODO Refine cylinder intersection point
		float phi = std::atan2(pHit.y, pHit.x);
		if (phi < 0) phi += 2 * Pi;
		if (pHit.z < zMin || pHit.z > zMax || phi > phiMax)
		{
			if (tShapeHit == t1) return false;
			if (t1.UpperBound() > ray.tMax) return false;
			pHit = ray(static_cast<float>(tShapeHit));
			// TODO Refine cylinder intersection point
			phi = std::atan2(pHit.y, pHit.x);
			if (phi < 0) phi += 2 * Pi;
			if (pHit.z < zMin || pHit.z > zMax || phi > phiMax) return false;
		}
		return true;
	}

	Interaction Cylinder::Sample(const Point2f& u) const
	{
		float z = Lerp(u[0], zMin, zMax);
		float phi = u[1] * phiMax;
		Point3f pObj = Point3f(radius * std::cos(phi), radius * std::sin(phi),z);
		Interaction it;
		it.n = Normalize((*ObjectToWorld)(Normal3f(pObj.x, pObj.y, 0)));
		if (reverseOrientation) it.n *= -1;
		float hitRad = std::sqrt(pObj.x * pObj.x + pObj.y * pObj.y);
		pObj.x *= radius / hitRad;
		pObj.y *= radius / hitRad;
		Vector3f pObjError = gamma(3) * Abs(Vector3f(pObj.x, pObj.y, 0));
		it.p = (*ObjectToWorld)(pObj, pObjError, &it.pError);
		return it;
	}


	float Cylinder::Area() const
	{
		return (zMax - zMin) * radius * phiMax;
	}

}