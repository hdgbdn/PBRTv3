#ifndef PBRT_TEXTURES_CONSTANT_H
#define PBRT_TEXTURES_CONSTANT_H

#include "core/pbrt.h"
#include "core/texture.h"

namespace pbrt
{
    template<typename T>
    class ConstantTexture : public Texture<T>
    {
    public:
        ConstantTexture(const T& value) : value(value) {}
        T Evaluate(const SurfaceInteraction&) const { return value; }
    private:
        T value;
    };
}

#endif