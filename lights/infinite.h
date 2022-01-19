#ifndef PBRT_LIGHT_INFINITE_H
#define PBRT_LIGHT_INFINITE_H

#include "light.h"
#include "mipmap.h"
#include "imageio.h"
#include "sampling.h"
namespace pbrt
{
	class InfiniteAreaLight : public Light
	{
	public:
		InfiniteAreaLight(const Transform& LightToWorld,
		                  const Spectrum& L, int nSamples, const std::string& texmap);
		Spectrum Sample_Li(const Interaction& ref, const Point2f& u, Vector3f* wi, float* pdf, VisibilityTester* vis) const override;
		float Pdf_Li(const Interaction& ref, const Vector3f& wi) const override;
		void Preprocess(const Scene& scene) override;
		Spectrum Power() const override;
		Spectrum Le(const RayDifferential& ray) const override;
	private:
		std::unique_ptr<MIPMap<RGBSpectrum>> Lmap;
		Point3f worldCenter;
		float worldRadius;
		std::unique_ptr<Distribution2D> distribution;
	};

    std::shared_ptr<InfiniteAreaLight> CreateInfinitedLight(const Transform& light2world, const ParamSet& paramSet);
}

#endif