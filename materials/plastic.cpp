#include "plastic.h"
#include "memory.h"
#include "interaction.h"
#include "microfacet.h"
#include "texture.h"
#include "paramset.h"

namespace pbrt
{
	void PlasticMaterial::ComputeScatteringFunctions(SurfaceInteraction* si, MemoryArena& arena, TransportMode mode, bool allowMultipleLobes) const
	{
		if (bumpMap)
			Bump(bumpMap, si);
		si->bsdf = ARENA_ALLOC(arena, BSDF)(*si);
		Spectrum kd = Kd->Evaluate(*si).Clamp();
		if (!kd.IsBlack())
			si->bsdf->Add(ARENA_ALLOC(arena, LambertianReflection)(kd));
		Spectrum ks = Ks->Evaluate(*si).Clamp();
		if (!ks.IsBlack())
		{
			Fresnel* fresnel = ARENA_ALLOC(arena, FresnelDielectric)(1.f, 1.5f);
			float rough = roughness->Evaluate(*si);
			if (remapRoughness)
				rough = TrowbridgeReitzDistribution::RoughnessToAlpha(rough);
			MicrofacetDistribution* distrib =
				ARENA_ALLOC(arena, TrowbridgeReitzDistribution)(rough, rough);
			BxDF* spec =
				ARENA_ALLOC(arena, MicrofacetReflection)(ks, distrib, fresnel);
			si->bsdf->Add(spec);
		}
	}

    PlasticMaterial *CreatePlasticMaterial(const TextureParams &mp)
    {
        auto Kd = mp.GetSpectrumTexture("Kd", Spectrum(0.25f));
        auto Ks = mp.GetSpectrumTexture("Ks", Spectrum(0.25f));
        auto roughness = mp.GetFloatTexture("roughness", .1f);
        auto bumpMap = mp.GetFloatTextureOrNull("bumpmap");
        bool remapRoughness = mp.FindBool("remapRoughness", true);
        return new PlasticMaterial(Kd, Ks, roughness, bumpMap, remapRoughness);
    }
}
