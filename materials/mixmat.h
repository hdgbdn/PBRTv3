#ifndef PBRT_MATERIALS_MIXMAT_H
#define PBRT_MATERIALS_MIXMAT_H

#include "core/pbrt.h"
#include "core/material.h"

namespace pbrt
{
	class MixMaterial : public Material
	{
	public:
		MixMaterial(const std::shared_ptr<Material>& m1,
		            const std::shared_ptr<Material>& m2,
		            const std::shared_ptr<Texture<Spectrum>>& scale)
			: m1(m1), m2(m2), scale(scale)
		{
		}

		void ComputeScatteringFunctions(SurfaceInteraction* si, MemoryArena& arena, TransportMode mode,
		                                bool allowMultipleLobes) const override;
	private:
		std::shared_ptr<Material> m1, m2;
		std::shared_ptr<Texture<Spectrum>> scale;
	};
}

#endif