#ifndef PBRT_MATERIALS_MATTE_H
#define PBRT_MATERIALS_MATTE_H

#include "pbrt.h"
#include "material.h"

namespace pbrt
{
    class MatteMaterial : public Material
    {
    public:
        MatteMaterial(const std::shared_ptr<Texture<Spectrum>>& Kd,
            const std::shared_ptr<Texture<float>>& sigma,
            const std::shared_ptr<Texture<float>>& bumpMap)
	            : Kd(Kd), sigma(sigma), bumpMap(bumpMap) { }

        void ComputeScatteringFunctions(SurfaceInteraction* si, MemoryArena& arena, TransportMode mode,
	        bool allowMultipleLobes) const override;
    private:
        std::shared_ptr<Texture<Spectrum>> Kd;
        std::shared_ptr<Texture<float>> sigma, bumpMap;
    };
}

#endif