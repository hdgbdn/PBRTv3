#include "scene.h"

namespace pbrt
{
    bool Scene::Intersect(const Ray &ray, SurfaceInteraction *isect) const
    {
        return aggregate->Intersect(ray, isect);
    }

    bool Scene::IntersectP(const Ray& ray) const
    {
        return aggregate->IntersectP(ray);
    }

    bool Scene::IntersectTr(Ray ray, Sampler &sampler, SurfaceInteraction *isect, Spectrum *Tr) const
    {
        // TODO implement
        return false;
    }
}

