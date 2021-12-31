#ifndef PBRT_CORE_INTEGRATOR_H
#define PBRT_CORE_INTEGRATOR_H

#include <utility>
#include "spectrum.h"
#include "sampling.h"


namespace pbrt
{
	Spectrum UniformSampleAllLights(const Interaction& it, const Scene& scene,
	                                MemoryArena& arena, Sampler& sampler,
	                                const std::vector<int>& nLightSamples,
	                                bool handleMedia = false);
	Spectrum UniformSampleOneLight(const Interaction& it, const Scene& scene,
	                               MemoryArena& arena, Sampler& sampler,
	                               bool handleMedia = false,
	                               const Distribution1D* lightDistrib = nullptr);

	Spectrum EstimateDirect(const Interaction& it,
		const Point2f& uScattering, const Light& light,
		const Point2f& uLight, const Scene& scene, Sampler& sampler,
		MemoryArena& arena, bool handleMedia = false, bool specular = false);

	class Integrator
	{
	public:
		~Integrator() {}
		virtual void Render(const Scene &scene) = 0;
	};
	class SamplerIntegrator: public Integrator
	{
	public:
		SamplerIntegrator(std::shared_ptr<const Camera> camera,
			std::shared_ptr<Sampler> sampler)
			: camera(std::move(camera)), sampler(std::move(sampler)) { }
		void Render(const Scene& scene) override;
		virtual void Preprocess(const Scene& scene, Sampler& sampler);
		virtual Spectrum Li(const RayDifferential& ray, const Scene& scene,
			Sampler& sampler, MemoryArena& arena, int depth = 0) const = 0;
		Spectrum SpecularReflect(const RayDifferential& ray,
			const SurfaceInteraction& isect,
			const Scene& scene, Sampler& sampler,
			MemoryArena& arena, int depth) const;
		Spectrum SpecularTransmit(const RayDifferential& ray,
			const SurfaceInteraction& isect,
			const Scene& scene, Sampler& sampler,
			MemoryArena& arena, int depth) const;

	protected:
	private:
		std::shared_ptr<const Camera> camera;
		std::shared_ptr<Sampler> sampler;
		
	};
}

#endif