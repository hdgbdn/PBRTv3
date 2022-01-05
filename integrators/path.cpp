#include "path.h"

#include "interaction.h"
#include "sampler.h"
#include "scene.h"

namespace pbrt
{
	PathIntegrator::PathIntegrator(int maxDepth, std::shared_ptr<const Camera> camera, std::shared_ptr<Sampler> sampler)
		: SamplerIntegrator(camera, sampler), maxDepth(maxDepth)
	{
	}
	Spectrum PathIntegrator::Li(const RayDifferential& r, const Scene& scene, Sampler& sampler, MemoryArena& arena, int depth) const
	{
		Spectrum L(0.f), beta(1.f);
		RayDifferential ray(r);
		bool specularBounce = false;
		for(int bounces = 0; ;++bounces)
		{
			SurfaceInteraction isect;
			bool foundIntersection = scene.Intersect(ray, &isect);
			if (bounces == 0 || specularBounce)
			{
				if (foundIntersection)
					L += beta * isect.Le(-ray.d);
				else
					for (const auto& light : scene.lights)
						L += beta * light->Le(ray);
			}
			if(!foundIntersection || bounces >= maxDepth)
				break;
			if (foundIntersection)
				L += beta * isect.Le(-ray.d);
			else
				for (const auto& light : scene.lights)
					L += beta * light->Le(ray);
			isect.ComputeScatteringFunctions(ray, arena, true);
			if(!isect.bsdf)
			{
				ray = isect.SpawnRay(ray.d);
				bounces--;
				continue;
			}
			L += beta * UniformSampleOneLight(isect, scene, arena, sampler);

			// Sample BSDF direction
			Vector3f wo = -ray.d, wi;
			float pdf;
			BxDFType flags;
			Spectrum f = isect.bsdf->Sample_f(wo, &wi, sampler.Get2D(), &pdf, BSDF_ALL, &flags);
			if (f.IsBlack() || pdf == 0.f)
				break;
			beta *= f * AbsDot(wi, isect.shading.n) / pdf;
			specularBounce = (flags & BSDF_SPECULAR) != 0;
			ray = isect.SpawnRay(wi);
			// TODO Account for subsurface scattering, if applicable

			// Russian roulette
			if (bounces > 3)
			{
				float q = std::max((float).05, 1 - beta.y());
				if (sampler.Get1D() < q)
					break;
				beta /= 1 - q;
					
			}
		}
		return L;
	}
}
