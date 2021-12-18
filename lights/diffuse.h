#ifndef PBRT_LIGHT_DIFFUSE_H
#define PBRT_LIGHT_DIFFUSE_H

#include "light.h"

namespace pbrt
{
	class DiffuseAreaLight : public AreaLight
	{
	public:
		DiffuseAreaLight(const Transform& LightToWorld,
			const MediumInterface& mediumInterface, const Spectrum& Lemit,
			int nSamples, const std::shared_ptr<Shape>& shape);
		Spectrum L(const Interaction& intr, const Vector3f& w) const override;
		Spectrum Power() const override;
	protected:
		const Spectrum Lemit;
		std::shared_ptr<Shape> shape;
		const float area;
	};
}

#endif