#ifndef PBRT_CORE_SAMPLING_H
#define PBRT_CORE_SAMPLING_H

#include "geometry.h"
#include "pbrt.h"
#include "rng.h"

namespace pbrt
{
    void StratifiedSample1D(float* samples, int nsamples, RNG& rng,
        bool jitter = true);
    void StratifiedSample2D(Point2f* samples, int nx, int ny, RNG& rng,
        bool jitter = true);
    void LatinHypercube(float* samples, int nSamples, int nDim, RNG& rng);
    template <typename T>
    void Shuffle(T* samp, int count, int nDimensions, RNG& rng) {
        for (int i = 0; i < count; ++i) {
            int other = i + rng.UniformUInt32(count - i);
            for (int j = 0; j < nDimensions; ++j)
                std::swap(samp[nDimensions * i + j], samp[nDimensions * other + j]);
        }
    }
}

#endif