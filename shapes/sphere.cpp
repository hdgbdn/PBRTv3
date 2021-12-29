#include "sphere.h"
#include "efloat.h"
#include "sampling.h"

namespace pbrt
{
	Sphere::Sphere(const std::shared_ptr<Transform>& ObjectToWorld, const std::shared_ptr<Transform>& WorldToObject, bool reverseOrientation, float radius, float zMin, float zMax, float phiMax)
		: Shape(ObjectToWorld, WorldToObject, reverseOrientation),
		radius(radius), zMin(Clamp(std::min(zMin, zMax), -radius, radius)),
		zMax(Clamp(std::max(zMin, zMax), -radius, radius)),
		thetaMin(std::acos(Clamp(zMin / radius, -1, 1))),
		thetaMax(std::acos(Clamp(zMax / radius, -1, 1))),
		phiMax(Radians(Clamp(phiMax, 0, 360))) {}

	Bounds3f Sphere::ObjectBound() const
	{
		return Bounds3f(Point3f(-radius, -radius, zMin),
			Point3f(radius, radius, zMax));
	}

	bool Sphere::Intersect(const Ray& r, float* tHit, SurfaceInteraction* isect, bool testAlphaTexture) const
	{
		Vector3f oErr, dErr;
		Ray ray = (*WorldToObject)(r, &oErr, &dErr);
		EFloat ox(ray.o.x, oErr.x), oy(ray.o.y, oErr.y), oz(ray.o.z, oErr.z);
		EFloat dx(ray.d.x, dErr.x), dy(ray.d.y, dErr.y), dz(ray.d.z, dErr.z);
		EFloat a = dx * dx + dy * dy + dz * dz;
		EFloat b = 2 * (ox * dx + oy * dy + oz * dz);
		EFloat c = ox * ox + oy * oy + oz * oz
			- static_cast<EFloat>(radius) * static_cast<EFloat>(radius);
		EFloat t0, t1;
		if (!Quadratic(a, b, c, &t0, &t1))
			return false;
		if (t0.UpperBound() > ray.tMax || t1.LowerBound() <= 0)
			return false;
		EFloat tShapeHit = t0;
		if(tShapeHit.LowerBound() <= 0)
		{
			tShapeHit = t1;
			if (tShapeHit.UpperBound() > ray.tMax)
				return false;
		}
		Point3f pHit = ray(static_cast<float>(tShapeHit));
		// TODO Refine sphere intersection point
		if (pHit.x == 0 && pHit.y == 0) pHit.x = 1e-5f * radius;
		float phi = std::atan2(pHit.x, pHit.y);
		if (phi < 0) phi += 2 * Pi;

		// Test sphere intersection against clipping parameters
		if((zMin > -radius && pHit.z < zMin) ||
			(zMax < radius && pHit.z > zMax) || phi > phiMax)
		{
			// if tShapeHit == t1, than it is the only root
			if (tShapeHit == t1) return false;
			// if the second root out of range, then don need to continue;
			if (t1.UpperBound() > ray.tMax) return false;
			tShapeHit = t1;
			pHit = ray(static_cast<float>(tShapeHit));
			// TODO Refine sphere intersection point
			if (pHit.x == 0 && pHit.y == 0) pHit.x = 1e-5f * radius;
			phi = std::atan2(pHit.y, pHit.x);
			// test again
			if ((zMin > -radius && pHit.z < zMin) ||
				(zMax < radius && pHit.z > zMax) || phi > phiMax)
				return false;
		}

		// Find parametric representation of sphere hit
		float u = phi / phiMax;
		float theta = std::acos(Clamp(pHit.z / radius, -1, 1));
		float v = (theta - thetaMin) / (thetaMax - thetaMin);

		float zRadius = std::sqrt(pHit.x * pHit.x + pHit.y * pHit.y);
		float invZRadius = 1 / zRadius;
		float cosPhi = pHit.x * invZRadius;
		float sinPhi = pHit.y * invZRadius;
		Vector3f dpdu(-phiMax * pHit.y, phiMax * pHit.x, 0);
		Vector3f dpdv = (thetaMax - thetaMin) *
			Vector3f(pHit.z * cosPhi, pHit.z * sinPhi, -radius * std::sin(theta));
		Normal3f dndu;
		Normal3f dndv;
		Vector3f pError;
		*isect = (*ObjectToWorld)(SurfaceInteraction(
		pHit, pError, Point2f(u, v), -ray.d, dpdu, dpdv, dndu, dndv, r.time, this));
		*tHit = static_cast<float>(tShapeHit);
		return true;
	}

	bool Sphere::IntersectP(const Ray& r, bool testAlphaTexture) const
	{
		Vector3f oErr, dErr;
		Ray ray = (*WorldToObject)(r, &oErr, &dErr);
		EFloat ox(ray.o.x, oErr.x), oy(ray.o.y, oErr.y), oz(ray.o.z, oErr.z);
		EFloat dx(ray.d.x, dErr.x), dy(ray.d.y, dErr.y), dz(ray.d.z, dErr.z);
		EFloat a = dx * dx + dy * dy + dz * dz;
		EFloat b = 2 * (ox * dx + oy * dy + oz * dz);
		EFloat c = ox * ox + oy * oy + oz * oz
			- static_cast<EFloat>(radius) * static_cast<EFloat>(radius);
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
		// TODO Refine sphere intersection point
		if (pHit.x == 0 && pHit.y == 0) pHit.x = 1e-5f * radius;
		float phi = std::atan2(pHit.x, pHit.y);
		if (phi < 0) phi += 2 * Pi;

		// Test sphere intersection against clipping parameters
		if ((zMin > -radius && pHit.z < zMin) ||
			(zMax < radius && pHit.z > zMax) || phi > phiMax)
		{
			// if tShapeHit == t1, than it is the only root
			if (tShapeHit == t1) return false;
			// if the second root out of range, then don need to continue;
			if (t1.UpperBound() > ray.tMax) return false;
			tShapeHit = t1;
			pHit = ray(static_cast<float>(tShapeHit));
			// TODO Refine sphere intersection point
			if (pHit.x == 0 && pHit.y == 0) pHit.x = 1e-5f * radius;
			phi = std::atan2(pHit.y, pHit.x);
			// test again
			if ((zMin > -radius && pHit.z < zMin) ||
				(zMax < radius && pHit.z > zMax) || phi > phiMax)
				return false;
		}
		return true;
	}

	Interaction Sphere::Sample(const Point2f& u) const
	{
		Point3f pObj = Point3f(0, 0, 0) + radius * UniformSampleSphere(u);
		Interaction it;
		it.n = Normalize((*ObjectToWorld)(Normal3f(pObj.x, pObj.y, pObj.z)));
		if (reverseOrientation) it.n *= -1;
		pObj *= radius / Distance(pObj, Point3f(0, 0, 0));
		Vector3f pObjError = gamma(5) * Abs(static_cast<Vector3f>(pObj));
			it.p = (*ObjectToWorld)(pObj, pObjError, &it.pError);
		return it;
	}

	Interaction Sphere::Sample(const Interaction& ref, const Point2f& u) const
	{
		Point3f pCenter = (*ObjectToWorld)(Point3f(0, 0, 0));
		Vector3f wc = Normalize(pCenter - ref.p);
		Vector3f wcX, wcY;
		CoordinateSystem(wc, &wcX, &wcY);
		Point3f pOrigin = OffsetRayOrigin(ref.p, ref.pError, ref.n, pCenter - ref.p);
		if (DistanceSquared(pOrigin, pCenter) <= radius * radius)
			return Sample(u);
		float sinThetaMax2 = radius * radius / DistanceSquared(ref.p, pCenter);
		float cosThetaMax = std::sqrt(std::max(static_cast<float>(0), 1 - sinThetaMax2));
		float cosTheta = (1 - u[0]) + u[0] * cosThetaMax;
		float sinTheta = std::sqrt(std::max(static_cast<float>(0), 1 - cosTheta * cosTheta));
		float phi = u[1] * 2 * Pi;
		float dc = Distance(ref.p, pCenter);
		float ds = dc * cosTheta - std::sqrt(std::max(static_cast<float>(0), radius * radius - dc * dc * sinTheta * sinTheta));
		float cosAlpha = (dc * dc + radius * radius - ds * ds) / (2 * dc * radius);
		float sinAlpha = std::sqrt(std::max(static_cast<float>(0), 1 - cosAlpha * cosAlpha));
		Vector3f nWorld =
			SphericalDirection(sinAlpha, cosAlpha, phi, -wcX, -wcY, -wc);
		Point3f pWorld = pCenter + radius * Point3f(nWorld.x, nWorld.y, nWorld.z);
		
		Interaction it;
		it.p = pWorld;
		it.pError = gamma(5) * Abs((Vector3f)pWorld);
		it.n = Normal3f(nWorld);
		if (reverseOrientation) it.n *= -1;

		return it;
	}

	float Sphere::Pdf(const Interaction& ref, const Vector3f& wi) const
	{
		Point3f pCenter = (*ObjectToWorld)(Point3f(0, 0, 0));
		Point3f pOrigin = OffsetRayOrigin(ref.p, ref.pError, ref.n, pCenter - ref.p);
		if (DistanceSquared(pOrigin, pCenter) <= radius * radius)
			return Shape::Pdf(ref, wi);
		float sinThetaMax2 = radius * radius / DistanceSquared(ref.p, pCenter);
		float cosThetaMax = std::sqrt(std::max((float)0, 1 - sinThetaMax2));
		return UniformConePdf(cosThetaMax);
	}

	float Sphere::Area() const
	{
		return phiMax * radius * (zMax - zMin);
	}


}
