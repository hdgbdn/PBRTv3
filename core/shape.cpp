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

	/// <summary>
	/// Interface for ray & shape intersection
	/// </summary>
	/// <param name="ray">
	///	Incoming ray, the intersections occur after ray.tMax will be ignored
	/// </param>
	/// <param name="tHit">
	/// The closest parametric distance
	/// </param>
	/// <param name="isect">
	/// Information about an intersection
	/// </param>
	/// <param name="testAlphaTexture"></param>
	/// <returns></returns>
	bool Shape::Intersect(const Ray& ray, float* tHit, SurfaceInteraction* isect, bool testAlphaTexture) const
	{
		
	}


}