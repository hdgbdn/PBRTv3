#ifndef PBRT_LIGHT_INFINITE_H
#define PBRT_LIGHT_INFINITE_H

#include "light.h"
#include "mipmap.h"
#include "imageio.h"
namespace pbrt
{
	class InfiniteAreaLight : public Light
	{
	public:
		InfiniteAreaLight(const Transform& LightToWorld,
		                  const Spectrum& L, int nSamples, const std::string& texmap);
		void Preprocess(const Scene& scene) override;
		Spectrum Power() const override;
		Spectrum Le(const RayDifferential& ray) const override;
	private:
		std::unique_ptr<MIPMap<RGBSpectrum>> Lmap;
		Point3f worldCenter;
		float worldRadius;
	};
}

#endif