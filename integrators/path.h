#ifndef PBRT_INTEGRATORS_PATH_H
#define PBRT_INTEGRATORS_PATH_H

#include "core/integrator.h"

namespace pbrt
{
	class PathIntegrator : public SamplerIntegrator
	{
	public:
		PathIntegrator(int maxDepth, std::shared_ptr<const Camera> camera, std::shared_ptr<Sampler> sampler,
			const Bounds2i& pixelBounds, float rrThreshold = 1,
			const std::string& lightSampleStrategy = "spatial");
		Spectrum Li(const RayDifferential& r, const Scene& scene, Sampler& sampler, MemoryArena& arena, int depth) const override;
	private:
		const int maxDepth;
		const float rrThreshold;
	};

	PathIntegrator* CreatePathIntegrator(const ParamSet& params,
		std::shared_ptr<Sampler> sampler,
		std::shared_ptr<const Camera> camera);
}

#endif