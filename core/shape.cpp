#include "pbrt.h"
#include "shape.h"

namespace pbrt
{
	Shape::Shape(const std::shared_ptr<Transform> ObjectToWorld, const std::shared_ptr<Transform> WorldToObject, bool reverseOrientation)
	: ObjectToWorld(ObjectToWorld), WorldToObject(WorldToObject),
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

}