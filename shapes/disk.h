#ifndef PBRT_SHAPE_DISK_H
#define PBRT_SHAPE_DISK_H

#include "pbrt.h"
#include "shape.h"

namespace pbrt
{
	class Disk : public Shape
	{
	public:
		Disk(const std::shared_ptr<Transform> ObjectToWorld,
			const std::shared_ptr<Transform> WorldToObject, bool reverseOrientation,
			float height, float radius, float innerRadius, float phiMax);
	bool Intersect(const Ray& ray, float* tHit, SurfaceInteraction* isect, bool testAlphaTexture) const override;
	bool IntersectP(const Ray& ray, bool testAlphaTexture) const override;
	Bounds3f ObjectBound() const override;
	float Area() override;
	private:
		const float height, radius, innerRadius, phiMax;
	};
}

#endif