#ifndef PBRT_CORE_MEDIUM_H
#define PBRT_CORE_MEDIUM_H

#include "pbrt.h"

namespace pbrt
{
    class Medium {
    public:
        virtual ~Medium() {}
        virtual Spectrum Tr(const Ray& ray, Sampler& sampler) const = 0;
        virtual Spectrum Sample(const Ray& ray, Sampler& sampler,
            MemoryArena& arena,
            MediumInteraction* mi) const = 0;
    };

    struct MediumInterface {
        MediumInterface() : inside(nullptr), outside(nullptr) {}
        MediumInterface(const Medium* medium) : inside(medium), outside(medium) {}
        MediumInterface(const Medium* inside, const Medium* outside)
            : inside(inside), outside(outside) {}
        bool IsMediumTransition() const { return inside != outside; }
        const Medium* inside, * outside;
    };
}
#endif // !#ifndef PBRT_CORE_MEDIUM_H