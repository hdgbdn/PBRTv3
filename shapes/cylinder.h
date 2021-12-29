#ifndef PBRT_SHAPE_CYLINDER_H
#define PBRT_SHAPE_CYLINDER_H

#include "pbrt.h"
#include "shape.h"

namespace pbrt
{
	class Cylinder : public Shape
	{
	public:
		Cylinder(const std::shared_ptr<Transform>& ObjectToWorld,
			const std::shared_ptr<Transform>& WorldToObject, bool reverseOrientation,
			float radius, float zMin, float zMax, float phiMax);
		Bounds3f ObjectBound() const override;
		bool Intersect(const Ray& ray, float* tHit, SurfaceInteraction* isect, bool testAlphaTexture) const override;
		bool IntersectP(const Ray& ray, bool testAlphaTexture) const override;
		float Area() const override;
		Interaction Sample(const Point2f& u) const override;
	protected:
		const float radius, zMin, zMax, phiMax;
	};
}

#endif