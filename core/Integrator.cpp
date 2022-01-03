#include "integrator.h"
#include "parallel.h"
#include "camera.h"
#include "memory.h"
#include "sampler.h"
#include "spectrum.h"
#include "interaction.h"
#include "scene.h"

namespace pbrt
{
	void SamplerIntegrator::Render(const Scene& scene)
	{
		Preprocess(scene, *sampler);
		// Render image tiles in parallel
		Bounds2i sampleBounds = camera->film->GetSampleBounds();
		Vector2i sampleExtent = sampleBounds.Diagonal();
		const int tileSize = 16;
		Point2i nTiles((sampleExtent.x + tileSize - 1) / tileSize,
			(sampleExtent.y + tileSize - 1) / tileSize);
		ParallelFor2D([&](Point2i tile) {
			// Allocate MemoryArena for tile
			MemoryArena arena;
			// Get sampler instance for tile
			int seed = tile.y * nTiles.x + tile.x;
			std::unique_ptr<Sampler> tileSampler(sampler->Clone(seed));
			// Compute sample bounds for tile
			int x0 = sampleBounds.pMin.x + tile.x * tileSize;
			int x1 = std::min(x0 + tileSize, sampleBounds.pMax.x);
			int y0 = sampleBounds.pMin.y + tile.y * tileSize;
			int y1 = std::min(y0 + tileSize, sampleBounds.pMax.y);
			Bounds2i tileBounds(Point2i(x0, y0), Point2i(x1, y1));
			// Get FilmTile for tile
			std::unique_ptr<FilmTile> filmTile = camera->film->GetFilmTile(tileBounds);
			// Loop over pixels in tile to render them
			for (auto pixel : tileBounds)
			{
				tileSampler->StartPixel(pixel);
				do
				{
					// Initialize CameraSample for current sample
					CameraSample cameraSample = tileSampler->GetCameraSample(pixel);
					// Generate camera ray for current sample
					RayDifferential ray;
					float rayWeight = camera->GenerateRayDifferential(cameraSample, &ray);
					ray.ScaleDifferentials(1 / std::sqrt(tileSampler->samplesPerPixel));
					// Evaluate radiance along camera ray
					Spectrum L(0.f);
					if (rayWeight > 0)
						L = Li(ray, scene, *tileSampler, arena);
					// Add camera ray’s contribution to image
					filmTile->AddSample(cameraSample.pFilm, L, rayWeight);
					// Free MemoryArena memory from computing image sample value
					arena.Reset();
				} while (tileSampler->StartNextSample());
			}
			// Merge image tile into Film
			camera->film->MergeFilmTile(std::move(filmTile));
			}, nTiles);
		//TODO Save final image after rendering
	}

	Spectrum SamplerIntegrator::SpecularReflect(const RayDifferential& ray, const SurfaceInteraction& isect, const Scene& scene, Sampler& sampler, MemoryArena& arena, int depth) const
	{
		// Compute specular reflection direction wi and BSDF value
		Vector3f wo = isect.wo, wi;
		float pdf;
		auto type = BxDFType(BSDF_REFLECTION | BSDF_SPECULAR);
		Spectrum f = isect.bsdf->Sample_f(wo, &wi, sampler.Get2D(), &pdf, type);
		// Return contribution of specular reflection
		const Normal3f& ns = isect.shading.n;
		if (pdf > 0 && !f.IsBlack() && AbsDot(wi, ns) != 0)
		{
			// Compute ray differential rd for specular reflection
			RayDifferential rd;
			return f * Li(rd, scene, sampler, arena, depth + 1) * AbsDot(wi, ns) / pdf;
		}
		else
			return { 0.f };
	}

	Spectrum UniformSampleAllLights(const Interaction& it, const Scene& scene, MemoryArena& arena, Sampler& sampler,
		const std::vector<int>& nLightSamples, bool handleMedia)
	{
		Spectrum L(0.f);
		for(size_t j = 0; j < scene.lights.size(); ++j)
		{
			const std::shared_ptr<Light>& light = scene.lights[j];
			int nSamples = nLightSamples[j];
			const Point2f* uLightArray = sampler.Get2DArray(nSamples);
			const Point2f* uScatteringArray = sampler.Get2DArray(nSamples);
			if(!uLightArray || !uScatteringArray)
			{
				Point2f uLight = sampler.Get2D();
				Point2f uScattering = sampler.Get2D();
				L += EstimateDirect(it, uScattering, *light, uLight, scene, sampler, arena, handleMedia);
			}
			else
			{
				Spectrum Ld(0.f);
				for (int k = 0; k < nSamples; ++k)
					Ld += EstimateDirect(it, uScatteringArray[k], *light, uLightArray[k],
						scene, sampler, arena, handleMedia);
				L += Ld / nSamples;
			}
		}
		return L;
	}

	Spectrum UniformSampleOneLight(const Interaction& it, const Scene& scene, MemoryArena& arena, Sampler& sampler, bool handleMedia, const Distribution1D* lightDistrib)
	{
		int nLights = int(scene.lights.size());
		if (nLights == 0) return { 0 };
		int lightNum = std::min((int)(sampler.Get1D() * nLights), nLights - 1);
		const std::shared_ptr<Light>& light = scene.lights[lightNum];
		Point2f uLight = sampler.Get2D();
		Point2f uScattering = sampler.Get2D();
		return (float)nLights *
			EstimateDirect(it, uScattering, *light, uLight, scene, sampler,
				arena, handleMedia);
	}

	Spectrum EstimateDirect(const Interaction& it, const Point2f& uScattering, const Light& light, const Point2f& uLight, const Scene& scene, Sampler& sampler, MemoryArena& arena, bool handleMedia, bool specular)
	{
		BxDFType bsdfFlags =
			specular ? BSDF_ALL : BxDFType(BSDF_ALL & ~BSDF_SPECULAR);
		Spectrum Ld(0.f);

		// Sample light
		Vector3f wi;
		float lightPdf = 0, scatteringPdf = 0;
		VisibilityTester visibility;
		Spectrum Li = light.Sample_Li(it, uLight, &wi, &lightPdf, &visibility);
		if (lightPdf > 0 && !Li.IsBlack())
		{
			Spectrum f;
			if (it.IsSurfaceInteraction())
			{
				const SurfaceInteraction& isect = static_cast<const SurfaceInteraction&>(it);
				f = isect.bsdf->f(isect.wo, wi, bsdfFlags) * AbsDot(wi, isect.shading.n);
				scatteringPdf = isect.bsdf->Pdf(isect.wo, wi, bsdfFlags);
			}
			else
			{
				// TODO volume scattering
			}

			if (!f.IsBlack())
			{
				if (handleMedia)
					Li *= visibility.Tr(scene, sampler);
				else if (!visibility.Unoccluded(scene))
					Li = Spectrum(0.f);
				if (!Li.IsBlack()) {
					if (IsDeltaLight(light.flags))
						Ld += f * Li / lightPdf;
					else {
						float weight = PowerHeuristic(1, lightPdf, 1, scatteringPdf);
						Ld += f * Li * weight / lightPdf;
					}
				}
			}
		}

		// Sample BSDF
		if (!IsDeltaLight(light.flags))
		{
			Spectrum f;
			bool sampledSpecular = false;
			if (it.IsSurfaceInteraction())
			{
				BxDFType sampledType;
				const SurfaceInteraction& isect = static_cast<const SurfaceInteraction&>(it);
				f = isect.bsdf->Sample_f(isect.wo, &wi, uScattering, &scatteringPdf,
					bsdfFlags, &sampledType);
				f *= AbsDot(wi, isect.shading.n);
				sampledSpecular = sampledType & BSDF_SPECULAR;
			}
			else
			{
				// TODO volume scattering
			}
			if (!f.IsBlack() && scatteringPdf > 0.f)
			{
				float weight = 1;
				if (!sampledSpecular)
				{
					lightPdf = light.Pdf_Li(it, wi);
					if (lightPdf == 0)
						return Ld;
					weight = PowerHeuristic(1, scatteringPdf, 1, lightPdf);
				}
				SurfaceInteraction lightIsect;
				Ray ray = it.SpawnRay(wi);
				Spectrum Tr(1.f);
				bool foundSurfaceInteraction = handleMedia
					? scene.IntersectTr(ray, sampler, &lightIsect, &Tr)
					: scene.Intersect(ray, &lightIsect);
				Spectrum Li(0.f);
				if (foundSurfaceInteraction)
				{
					if (lightIsect.primitive->GetAreaLight() == &light)
						Li = lightIsect.Le(-wi);
				}
				else
					Li = light.Le(ray);
				if (!Li.IsBlack())
					Ld += f * Li * Tr * weight / scatteringPdf;
			}
		}
		return Ld;
	}
}
