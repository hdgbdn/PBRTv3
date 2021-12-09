#ifndef PBRT_MATERIALS_PLASTIC_H
#define PBRT_MATERIALS_PLASTIC_H

#include "material.h"
#include "pbrt.h"

namespace pbrt
{
	class PlasticMaterial : public Material
	{
	public:
		PlasticMaterial(const std::shared_ptr<Texture<Spectrum>>& Kd,
		                const std::shared_ptr<Texture<Spectrum>>& Ks,
		                const std::shared_ptr<Texture<float>>& roughness,
		                const std::shared_ptr<Texture<float>>& bumpMap,
		                bool remapRoughness)
			: Kd(Kd), Ks(Ks), roughness(roughness), bumpMap(bumpMap), remapRoughness(remapRoughness){}
		void ComputeScatteringFunctions(SurfaceInteraction* si, MemoryArena& arena, TransportMode mode, bool allowMultipleLobes) const override;
	private:
		std::shared_ptr<Texture<Spectrum>> Kd, Ks;
		std::shared_ptr<Texture<float>> roughness, bumpMap;
		const bool remapRoughness;
	};
}


#endif