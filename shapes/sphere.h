#ifndef PBRT_SHAPE_SPHERE_H
#define PBRT_SHAPE_SPHERE_H

#include "core/shape.h"

namespace pbrt
{
	class Sphere : public Shape
	{
	public:
		Sphere(const Transform* ObjectToWorld,
               const Transform* WorldToObject, bool reverseOrientation,
			float radius, float zMin, float zMax, float phiMax);
		Bounds3f ObjectBound() const override;
		bool Intersect(const Ray& ray, float* tHit, SurfaceInteraction* isect, bool testAlphaTexture) const override;
		bool IntersectP(const Ray& ray, bool testAlphaTexture) const override;
		float Area() const override;
		Interaction Sample(const Point2f& u) const override;
		Interaction Sample(const Interaction& ref, const Point2f& u) const override;
		float Pdf(const Interaction& ref, const Vector3f& wi) const override;
		const float radius;
		const float zMin, zMax;
		const float thetaMin, thetaMax, phiMax;
	};

    std::shared_ptr<Shape> CreateSphereShape(const Transform *o2w,
                                             const Transform *w2o,
                                             bool reverseOrientation,
                                             const ParamSet &params);
}

#endif