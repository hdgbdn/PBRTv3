#ifndef PBRT_INTEGRATORS_DIRECTLIGHTING_H
#define PBRT_INTEGRATORS_DIRECTLIGHTING_H

#include "integrator.h"
#include "scene.h"

namespace pbrt
{
	enum class LightStrategy { UniformSampleAll, UniformSampleOne };

	class DirectLightingIntegrator : public SamplerIntegrator
	{
	public:
		DirectLightingIntegrator(LightStrategy strategy, int maxDepth,
			std::shared_ptr<const Camera> camera,
			std::shared_ptr<Sampler> sampler);
		void Preprocess(const Scene& scene, Sampler& sampler) override;
		Spectrum Li(const RayDifferential& ray, const Scene& scene, Sampler& sampler, MemoryArena& arena, int depth) const override;
	private:
		const LightStrategy strategy;
		const int maxDepth;
		std::vector<int> nLightSamples;
	};
}

#endif