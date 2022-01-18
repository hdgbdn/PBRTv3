#ifndef PBRT_MATERIALS_MATTE_H
#define PBRT_MATERIALS_MATTE_H

#include <utility>

#include "pbrt.h"
#include "material.h"

namespace pbrt
{
    class MatteMaterial : public Material
    {
    public:
        MatteMaterial(std::shared_ptr<Texture<Spectrum>>  Kd,
            std::shared_ptr<Texture<float>>  sigma,
            std::shared_ptr<Texture<float>>  bumpMap)
	            : Kd(std::move(Kd)), sigma(std::move(sigma)), bumpMap(std::move(bumpMap)) { }

        void ComputeScatteringFunctions(SurfaceInteraction* si, MemoryArena& arena, TransportMode mode,
	        bool allowMultipleLobes) const override;
    private:
        std::shared_ptr<Texture<Spectrum>> Kd;
        std::shared_ptr<Texture<float>> sigma, bumpMap;
    };

    MatteMaterial* CreateMatteMaterial(const TextureParams& mp);
}

#endif