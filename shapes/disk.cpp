#include "disk.h"
#include "efloat.h"

namespace pbrt
{
	Disk::Disk(const std::shared_ptr<Transform> ObjectToWorld, const std::shared_ptr<Transform> WorldToObject, bool reverseOrientation, float height, float radius, float innerRadius, float phiMax)
		: Shape(ObjectToWorld, WorldToObject, reverseOrientation),
		height(height), radius(radius), innerRadius(innerRadius), phiMax(phiMax) {}

	Bounds3f Disk::ObjectBound() const
	{
		return Bounds3f(Point3f(-radius, -radius, height),
			Point3f(radius, radius, height));
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
		if()
	}


}