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

}
