#ifndef PBRT_CORE_SHAPE_H
#define PBRT_CORE_SHAPE_H

#include "pbrt.h"
#include "transformation.h"

namespace pbrt
{
	class Shape
	{
	public:
		Shape(const std::shared_ptr<Transform> ObjectToWorld,
			const std::shared_ptr<Transform> WorldToObject, bool reverseOrientation);
		virtual ~Shape();
		virtual Bounds3f ObjectBound() const = 0;
		virtual Bounds3f WorldBound() const;
		virtual bool Intersect(const Ray& ray, float* tHit,
			SurfaceInteraction* isect, bool testAlphaTexture = true) const = 0;
		const std::shared_ptr<Transform> ObjectToWorld;
		const std::shared_ptr<Transform> WorldToObject;
		const bool reverseOrientation;
		const bool transformSwapsHandedness;
	};
}

#endif