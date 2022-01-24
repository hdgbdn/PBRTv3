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
			int nSamples, const std::shared_ptr<Shape>& shape, bool twoSided = false);
		Spectrum L(const Interaction& intr, const Vector3f& w) const override;
		Spectrum Power() const override;
		Spectrum Sample_Li(const Interaction& ref, const Point2f& u, Vector3f* wi, float* pdf, VisibilityTester* vis) const override;
		float Pdf_Li(const Interaction& ref, const Vector3f& wi) const override;
	protected:
		const Spectrum Lemit;
		std::shared_ptr<Shape> shape;
		const float area;
	};
	std::shared_ptr<AreaLight> CreateDiffuseAreaLight(
		const Transform& light2world, const Medium* medium,
		const ParamSet& paramSet, const std::shared_ptr<Shape>& shape);
}

#endif