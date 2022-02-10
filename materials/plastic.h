#ifndef PBRT_MATERIALS_PLASTIC_H
#define PBRT_MATERIALS_PLASTIC_H

#include <utility>

#include "core/pbrt.h"
#include "core/material.h"

namespace pbrt
{
	class PlasticMaterial : public Material
	{
	public:
		PlasticMaterial(std::shared_ptr<Texture<Spectrum>>  Kd,
		                std::shared_ptr<Texture<Spectrum>>  Ks,
		                std::shared_ptr<Texture<float>>  roughness,
		                std::shared_ptr<Texture<float>>  bumpMap,
		                bool remapRoughness)
			: Kd(std::move(Kd)), Ks(std::move(Ks)), roughness(std::move(roughness)), bumpMap(std::move(bumpMap)), remapRoughness(remapRoughness){}
		void ComputeScatteringFunctions(SurfaceInteraction* si, MemoryArena& arena, TransportMode mode, bool allowMultipleLobes) const override;
	private:
		std::shared_ptr<Texture<Spectrum>> Kd, Ks;
		std::shared_ptr<Texture<float>> roughness, bumpMap;
		const bool remapRoughness;
	};

    PlasticMaterial* CreatePlasticMaterial(const TextureParams& mp);
}


#endif