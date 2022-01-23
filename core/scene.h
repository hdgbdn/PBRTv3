#ifndef PBRT_CORE_SCENE_H
#define PBRT_CORE_SCENE_H

#include "pbrt.h"
#include "primitive.h"
#include "light.h"
#include "geometry.h"

namespace pbrt
{
	class Scene
	{
	public:
		Scene(std::shared_ptr<Primitive> aggregate,
			const std::vector<std::shared_ptr<Light>>& light)
			: aggregate(aggregate), lights(light),
				worldBound(aggregate->WorldBound())
		{
			for (const auto& light : lights)
				light->Preprocess(*this);
		}
		std::vector<std::shared_ptr<Light>> lights;
		const Bounds3f Worldbound() const { return worldBound; }
		bool Intersect(const Ray& ray, SurfaceInteraction* isect) const;
		bool IntersectP(const Ray& ray) const;
		bool IntersectTr(Ray ray, Sampler& sampler,
			SurfaceInteraction* isect, Spectrum* Tr) const;
	private:
		std::shared_ptr<Primitive> aggregate;
		Bounds3f worldBound;
	};
}

#endif