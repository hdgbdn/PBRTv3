#ifndef PBRT_CORE_INTERACTION_H
#define PBRT_CORE_INTERACTION_H

#include "geometry.h"
#include "material.h"
#include "reflection.h"

namespace pbrt
{
	struct Interaction
	{
	public:
		Interaction() : time(0) {}
		float time;
		Point3f p;
		Vector3f wo;
		Normal3f n;
	};
	class SurfaceInteraction : public Interaction
	{
	public:
		SurfaceInteraction() {}
		void ComputeScatteringFunctions(
			const RayDifferential& ray, MemoryArena& arena,
			bool allowMultipleLobes = false,
			TransportMode mode = TransportMode::Radiance);
		Spectrum Le(const Vector3f& w) const;
		Point2f uv;
		Vector3f dpdu, dpdv;
		Normal3f dndu, dndv;
		struct {
			Normal3f n;
			Vector3f dpdu, dpdv;
			Normal3f dndu, dndv;
		} shading;
		BSDF* bsdf = nullptr;
		BSSRDF* bssrdf = nullptr;
	};
}

#endif