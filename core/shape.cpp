#include "pbrt.h"
#include "shape.h"

#include <utility>

namespace pbrt
{
	Shape::Shape(std::shared_ptr<Transform> ObjectToWorld, std::shared_ptr<Transform> WorldToObject, bool reverseOrientation)
	: ObjectToWorld(std::move(ObjectToWorld)), WorldToObject(std::move(WorldToObject)),
		reverseOrientation(reverseOrientation),
		transformSwapsHandedness(ObjectToWorld->SwapsHandedness()) {}

	Bounds3f Shape::WorldBound() const
	{
		return (*ObjectToWorld)(ObjectBound());
	}

	bool Shape::IntersectP(const Ray& ray, bool testAlphaTexture) const
	{
		float tHit = ray.tMax;
		SurfaceInteraction isect;
		return Intersect(ray, &tHit, &isect, testAlphaTexture);
	}

	float Shape::Pdf(const Interaction& ref, const Vector3f& wi) const
	{
		Ray ray = ref.SpawnRay(wi);
		float tHit;
		SurfaceInteraction isectLight;
		if (!Intersect(ray, &tHit, &isectLight, false)) return 0;
		float pdf = DistanceSquared(ref.p, isectLight.p) /
			(AbsDot(isectLight.n, -wi) * Area());
		return 0.0f;
	}

}