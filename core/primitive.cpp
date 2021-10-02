#include "primitive.h"
#include "material.h"

namespace pbrt
{
	GeometricPrimitive::GeometricPrimitive(const std::shared_ptr<Shape>& shape,
	                                       const std::shared_ptr<Material>& material,
	                                       const std::shared_ptr<AreaLight>& areaLight,
	                                       const MediumInterface& mediumInterface)
		: shape(shape), material(material), areaLight(areaLight),
		  mediumInterface(mediumInterface){}

	Bounds3f GeometricPrimitive::WorldBound() const
	{
		return shape->WorldBound();
	}

	bool GeometricPrimitive::Intersect(const Ray& r, SurfaceInteraction* isect) const
	{
		float tHit;
		if (!shape->Intersect(r, &tHit, isect)) return false;
		r.tMax = tHit;
		isect->primitive = this;
		// TODO Initialize SurfaceInteraction::mediumInterface after Shape intersection
		return true;
	}

	bool GeometricPrimitive::IntersectP(const Ray& r)
	{
		return shape->IntersectP(r);
	}

	const AreaLight* GeometricPrimitive::GetAreaLight() const
	{
		return areaLight.get();
	}

	const Material* GeometricPrimitive::GetMaterial() const
	{
		return material.get();
	}

	void GeometricPrimitive::ComputeScatteringFunctions(SurfaceInteraction* isect, MemoryArena& arena,
	                                                    TransportMode mode, bool allowMultipleLobes) const
	{
		if (material)
			material->ComputeScatteringFunctions(isect, arena, mode, allowMultipleLobes);
	}

}
