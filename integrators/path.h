#ifndef PBRT_INTEGRATORS_PATH_H
#define PBRT_INTEGRATORS_PATH_H

#include "integrator.h"

namespace pbrt
{
	class PathIntegrator : public SamplerIntegrator
	{
	public:
		PathIntegrator(int maxDepth, std::shared_ptr<const Camera> camera, std::shared_ptr<Sampler> sampler);
		Spectrum Li(const RayDifferential& r, const Scene& scene, Sampler& sampler, MemoryArena& arena, int depth) const override;
	private:
		const int maxDepth;
	};
}

#endif