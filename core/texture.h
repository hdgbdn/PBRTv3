#ifndef PBRT_CORE_TEXTURE_H
#define PBRT_CORE_TEXTURE_H

namespace pbrt
{
    template <typename T>
    class Texture {
    public:
        virtual T Evaluate(const SurfaceInteraction&) const = 0;
        virtual ~Texture() {}
    };
}

#endif