#ifndef PBRT_CORE_PRIMITIVE_H
#define PBRT_CORE_PRIMITIVE_H

#include "pbrt.h"
#include "geometry.h"

namespace pbrt
{
	class Primitive
	{
	public:
		const Bounds3f& WorldBound() const;
		bool IntersectP(const Ray&, SurfaceInteraction* isect);
	};
}

#endif