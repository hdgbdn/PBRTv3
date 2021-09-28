#ifndef PBRT_SHAPE_SPHERE_H
#define PBRT_SHAPE_SPHERE_H

#include "pbrt.h"
#include "transformation.h"
#include "shape.h"

namespace pbrt
{
	class Sphere : public Shape
	{
	public:
		Sphere(const std::shared_ptr<Transform>& ObjectToWorld,
			const std::shared_ptr<Transform>& WorldToObject, bool reverseOrientation,
			float radius, float zMin, float zMax, float phiMax);
		Bounds3f ObjectBound() const override;
		bool Intersect(const Ray& ray, float* tHit, SurfaceInteraction* isect, bool testAlphaTexture) const override;
		bool IntersectP(const Ray& ray, bool testAlphaTexture) const override;
		float Area() override;
		const float radius;
		const float zMin, zMax;
		const float thetaMin, thetaMax, phiMax;
	};
}

#endif