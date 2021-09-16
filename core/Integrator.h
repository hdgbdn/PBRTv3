#ifndef PBRT_CORE_INTEGRATOR_H
#define PBRT_CORE_INTEGRATOR_H

#include "pbrt.h"
#include "geometry.h"
#include "spectrum.h"


namespace pbrt
{
	class Integrator
	{
	public:
		virtual void Render(const Scene &scene) = 0;
	};
	class SamplerIntegrator: public Integrator
	{
	public:
		SamplerIntegrator(std::shared_ptr<const Camera>,
			std::shared_ptr<Sampler> sampler);
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
		std::shared_ptr<Sampler> sampler;
		std::shared_ptr<const Camera> camera;
		
	};
}

#endif