#include "infinite.h"
#include "scene.h"
#include "paramset.h"

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

		// Compute scalar-valued image img from environment map
		int width = resolution.x, height = resolution.y;
		float filter = (float)1 / std::max(width, height);
		std::unique_ptr<float[]> img(new float[width * height]);
		for(int v = 0; v < height; ++v)
		{
			float vp = (float)v / (float)height;
			float sinTheta = std::sin(Pi * float(v + .5) / float(height));
			for (int u = 0; u < width; ++u) {
				float up = (float)u / (float)width;
				img[u + v * width] = Lmap->Lookup(Point2f(up, vp), filter).y();
				img[u + v * width] *= sinTheta;
			}
		}
		distribution.reset(new Distribution2D(img.get(), width, height));
	}
	Spectrum InfiniteAreaLight::Sample_Li(const Interaction& ref, const Point2f& u, Vector3f* wi, float* pdf, VisibilityTester* vis) const
	{
		float mapPdf;
		Point2f uv = distribution->SampleContinuous(u, &mapPdf);
		if (mapPdf == 0) return { 0 };
		float theta = uv[1] * Pi, phi = uv[0] * 2 * Pi;
		float cosTheta = std::cos(theta), sinTheta = std::sin(theta);
		float cosPhi = std::cos(phi), sinPhi = std::sin(phi);
		*wi = LightToWorld(Vector3f(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta));
		if (sinTheta == 0) *pdf = 0;
		else *pdf = mapPdf / (2 * Pi * Pi * sinTheta);
		*vis = VisibilityTester(ref, Interaction(ref.p + *wi * (2 * worldRadius),
			ref.time, mediumInterface));
		return { Lmap->Lookup(uv), SpectrumType::Illuminant };
 	}
	float InfiniteAreaLight::Pdf_Li(const Interaction& ref, const Vector3f& wi) const
	{
		Vector3f wiL = WorldToLight(wi);
		float theta = SphericalTheta(wiL), phi = SphericalPhi(wiL);
		float sinTheta = std::sin(theta);
		if (sinTheta == 0) return 0;
		return distribution->Pdf(Point2f(phi * Inv2Pi, theta * InvPi)) /
			(2 * Pi * Pi * sinTheta);
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

    std::shared_ptr<InfiniteAreaLight> CreateInfinitedLight(const Transform &light2world, const ParamSet &paramSet)
    {
        Spectrum L = paramSet.FindOneSpectrum("L", Spectrum(1.0f));
        Spectrum sc = paramSet.FindOneSpectrum("scale", Spectrum(1.0f));
        std::string texmap = paramSet.FindOneFilename("mapname", "");
        int nSamples = paramSet.FindOneInt("samples", paramSet.FindOneInt("nsamples", 1));
        return std::make_shared<InfiniteAreaLight>(light2world, L * sc, nSamples,
                                                   texmap);
    }
}
