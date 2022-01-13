#include "scene.h"

namespace pbrt
{
    bool Scene::Intersect(const Ray &ray, SurfaceInteraction *isect) const
    {
        // TODO implement
        return false;
    }

    bool Scene::IntersectTr(Ray ray, Sampler &sampler, SurfaceInteraction *isect, Spectrum *Tr) const
    {
        // TODO implement
        return false;
    }
}

