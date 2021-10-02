#ifndef PBRT_CORE_PRIMITIVE_H
#define PBRT_CORE_PRIMITIVE_H

#include "pbrt.h"
#include "geometry.h"
#include "material.h"
#include "interaction.h"

namespace pbrt
{
	class Primitive
	{
	public:
		virtual Bounds3f WorldBound() const = 0;
		virtual bool Intersect(const Ray& r, SurfaceInteraction* isect) const = 0;
		virtual bool IntersectP(const Ray&);
		virtual const AreaLight* GetAreaLight() const = 0;
		virtual const Material* GetMaterial() const = 0;
		virtual void ComputeScatteringFunctions(SurfaceInteraction* isect,
			MemoryArena& arena, TransportMode mode, bool allowMultipleLobes) const = 0;
		BSDF* bsdf = nullptr;
		BSSRDF* bssrdf = nullptr;
	};

	class GeometricPrimitive : public Primitive
	{
	public:
		GeometricPrimitive(const std::shared_ptr<Shape>& shape,
			const std::shared_ptr<Material>& material,
			const std::shared_ptr<AreaLight>& areaLight,
			const MediumInterface& mediumInterface);
		Bounds3f WorldBound() const override;
		bool Intersect(const Ray& r, SurfaceInteraction* isect) const override;
		bool IntersectP(const Ray&) override;
		const AreaLight* GetAreaLight() const override;
		const Material* GetMaterial() const override;
	void ComputeScatteringFunctions(SurfaceInteraction* isect, MemoryArena& arena, TransportMode mode, bool allowMultipleLobes) const override;
	private:
		std::shared_ptr<Shape> shape;
		std::shared_ptr<Material> material;
		std::shared_ptr<AreaLight> areaLight;
		MediumInterface mediumInterface;
	};
}

#endif