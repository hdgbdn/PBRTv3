#include "disk.h"
#include "efloat.h"
#include "paramset.h"
#include "sampling.h"

namespace pbrt
{
	Disk::Disk(const Transform *ObjectToWorld, const Transform *WorldToObject, bool reverseOrientation, float height, float radius, float innerRadius, float phiMax)
		: Shape(ObjectToWorld, WorldToObject, reverseOrientation),
		height(height), radius(radius), innerRadius(innerRadius), phiMax(phiMax) {}

	Bounds3f Disk::ObjectBound() const
	{
		return Bounds3f(Point3f(-radius, -radius, height),
			Point3f(radius, radius, height));
	}

	float Disk::Area() const
	{
		return phiMax * 0.5 * (radius * radius - innerRadius * innerRadius);
	}

	Interaction Disk::Sample(const Point2f& u) const
	{
		Point2f pd = ConcentricSampleDisk(u);
		Point3f pObj(pd.x * radius, pd.y * radius, height);
		Interaction it;
		it.n = Normalize((*ObjectToWorld)(Normal3f(0, 0, 1)));
		if (reverseOrientation) it.n *= -1;
		it.p = (*ObjectToWorld)(pObj, Vector3f(0, 0, 0), &it.pError);
		return it;
	}

	std::shared_ptr<Disk> CreateDiskShape(const Transform* o2w, const Transform* w2o, bool reverseOrientation,
		const ParamSet& params)
	{
		float height = params.FindOneFloat("height", 0.);
		float radius = params.FindOneFloat("radius", 1);
		float inner_radius = params.FindOneFloat("innerradius", 0);
		float phimax = params.FindOneFloat("phimax", 360);
		return std::make_shared<Disk>(o2w, w2o, reverseOrientation, height, radius,
			inner_radius, phimax);
	}

	bool Disk::Intersect(const Ray& r, float* tHit, SurfaceInteraction* isect, bool testAlphaTexture) const
	{
		Vector3f oErr, dErr;
		Ray ray = (*WorldToObject)(r, &oErr, &dErr);
		EFloat ox(ray.o.x, oErr.x), oy(ray.o.y, oErr.y), oz(ray.o.z, oErr.z);
		EFloat dx(ray.d.x, dErr.x), dy(ray.d.y, dErr.y), dz(ray.d.z, dErr.z);
		if (ray.d.z == 0) return false;
		float tShapeHit = (height - ray.o.z) / ray.d.z;
		if (tShapeHit <= 0 || tShapeHit >= ray.tMax) return false;
		Point3f pHit = ray(tShapeHit);
		float distSqrt = pHit.x * pHit.x + pHit.y * pHit.y;
		if (distSqrt > radius * radius || distSqrt < innerRadius * innerRadius)
			return false;
		float phi = std::atan2(pHit.y, pHit.x);
		if (phi < 0) phi += 2 * Pi;
		if (phi > phiMax) return false;
		float u = phi / phiMax;
		float rHit = std::sqrt(distSqrt);
		float oneMinusV = ((rHit - innerRadius) / (radius - innerRadius));
		float v = 1 - oneMinusV;
		Vector3f dpdu(-phiMax * pHit.y, phiMax * pHit.x, 0);
		Vector3f dpdv = Vector3f(pHit.x, pHit.y, 0.f)
			* (innerRadius - radius) / rHit;
		Normal3f dndu(0, 0, 0), dndv(0, 0, 0);

		// Refine disk intersection point
		pHit.z = height;

		Vector3f pError(0, 0, 0);
		*isect = (*ObjectToWorld)(SurfaceInteraction(pHit, pError, Point2f(u, v),
			-ray.d, dpdu, dpdv, dndu, dndv,
			ray.time, this));
		*tHit = tShapeHit;
		return true;
	}

	bool Disk::IntersectP(const Ray& r, bool testAlphaTexture) const
	{
		Vector3f oErr, dErr;
		Ray ray = (*WorldToObject)(r, &oErr, &dErr);
		EFloat ox(ray.o.x, oErr.x), oy(ray.o.y, oErr.y), oz(ray.o.z, oErr.z);
		EFloat dx(ray.d.x, dErr.x), dy(ray.d.y, dErr.y), dz(ray.d.z, dErr.z);
		if (ray.d.z == 0) return false;
		float tShapeHit = (height - ray.o.z) / ray.d.z;
		if (tShapeHit <= 0 || tShapeHit >= ray.tMax) return false;
		Point3f pHit = ray(tShapeHit);
		float distSqrt = pHit.x * pHit.x + pHit.y * pHit.y;
		if (distSqrt > radius * radius || distSqrt < innerRadius * innerRadius)
			return false;
		float phi = std::atan2(pHit.y, pHit.x);
		if (phi < 0) phi += 2 * Pi;
		if (phi > phiMax) return false;
		return true;
	}
}