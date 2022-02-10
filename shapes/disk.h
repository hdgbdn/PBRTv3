#ifndef PBRT_SHAPE_DISK_H
#define PBRT_SHAPE_DISK_H

#include "core/pbrt.h"
#include "core/shape.h"

namespace pbrt
{
	class Disk : public Shape
	{
	public:
		Disk(const Transform *ObjectToWorld,
             const Transform *WorldToObject, bool reverseOrientation,
             float height, float radius, float innerRadius, float phiMax);
		bool Intersect(const Ray& ray, float* tHit, SurfaceInteraction* isect, bool testAlphaTexture) const override;
		bool IntersectP(const Ray& ray, bool testAlphaTexture) const override;
		Bounds3f ObjectBound() const override;
		float Area() const override;
		Interaction Sample(const Point2f& u) const override;
	private:
		const float height, radius, innerRadius, phiMax;
	};

	std::shared_ptr<Disk> CreateDiskShape(const Transform* o2w,
		const Transform* w2o,
		bool reverseOrientation,
		const ParamSet& params);
}

#endif