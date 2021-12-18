#include "infinite.h"

namespace pbrt
{
	InfiniteAreaLight::InfiniteAreaLight(const Transform& LightToWorld, const Spectrum& L, int nSamples, const std::string& texmap)
		:Light((int)LightFlags::Infinite, LightToWorld,
			MediumInterface(), nSamples)
	{
		Point2i resolution;
		std::unique_ptr<RGBSpectrum[]> texels(nullptr);
		if(texmap != "")
		{
			texels = ReadImage(texmap, &resolution);
			if (texels)
				for (int i = 0; i < resolution.x * resolution.y; ++i)
					texels[i] *= L.ToRGBSpectrum();
		}
		if(!texels)
		{
			resolution.x = resolution.y = 1;
			texels = std::unique_ptr<RGBSpectrum[]>(new RGBSpectrum[1]);
			texels[0] = L.ToRGBSpectrum();
		}
		Lmap.reset(new MIPMap<RGBSpectrum>(resolution, texels.get()));
		//TODO maybe in monte carlo? Initialize sampling PDFs for infinite area light

	}
	void InfiniteAreaLight::Preprocess(const Scene& scene)
	{
		scene.Worldbound().BoundingSphere(&worldCenter, &worldRadius);
	}
	Spectrum InfiniteAreaLight::Power() const
	{
		return Pi * worldRadius * worldRadius *
			Spectrum(Lmap->Lookup(Point2f(.5f, .5f), .5f),
				SpectrumType::Illuminant);
	}

	Spectrum InfiniteAreaLight::Le(const RayDifferential& ray) const
	{
		Vector3f w = Normalize(WorldToLight(ray.d));
		Point2f st(SphericalPhi(w) * Inv2Pi,
			SphericalTheta(w) * InvPi);
		return Spectrum(Lmap->Lookup(st), SpectrumType::Illuminant);
	}
}
