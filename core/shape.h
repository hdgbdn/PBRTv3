#ifndef PBRT_CORE_SHAPE_H
#define PBRT_CORE_SHAPE_H

#include "transformation.h"
#include "interaction.h"

namespace pbrt
{
	class Shape
	{
	public:
		Shape(const std::shared_ptr<Transform>& ObjectToWorld,
			const std::shared_ptr<Transform>& WorldToObject, bool reverseOrientation);
		virtual ~Shape() = default;
		virtual Bounds3f ObjectBound() const = 0;
		virtual Bounds3f WorldBound() const;
		virtual bool IntersectP(const Ray& ray,
			bool testAlphaTexture = true) const;
		virtual bool Intersect(const Ray& ray, float* tHit,
			SurfaceInteraction* isect, bool testAlphaTexture = true) const = 0;
		virtual float Area() const = 0;
		virtual Interaction Sample(const Point2f& u) const = 0;
		virtual Interaction Sample(const Interaction& ref, const Point2f& u) const { return Sample(u); }
		virtual float Pdf(const Interaction&) const { return 1 / Area(); }
		virtual float Pdf(const Interaction& ref, const Vector3f& wi) const;
		const std::shared_ptr<Transform> ObjectToWorld;
		const std::shared_ptr<Transform> WorldToObject;
		const bool reverseOrientation;
		const bool transformSwapsHandedness;
	};
}

#endif