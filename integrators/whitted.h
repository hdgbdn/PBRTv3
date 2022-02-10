#ifndef PBRT_INTEGRATORS_WHITTED_H
#define PBRT_INTEGRATORS_WHITTED_H

#include "core/pbrt.h"
#include "core/integrator.h"

namespace pbrt
{
	class WhittedIntegrator : public SamplerIntegrator
	{
	public:
		Spectrum Li(const RayDifferential& ray, const Scene& scene, Sampler& sampler, MemoryArena& arena, int depth) const override;
	private:
		const int maxDepth;
	};
}

#endif
