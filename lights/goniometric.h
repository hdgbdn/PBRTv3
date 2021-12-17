#ifndef PBRT_LIGHTS_GONIOMETRIC_H
#define PBRT_LIGHTS_GONIOMETRIC_H

#include "light.h"
#include "mipmap.h"
#include "imageio.h"

namespace pbrt
{
	class GonioPhotometricLight : public Light
	{
	public:
		GonioPhotometricLight(const Transform& LightToWorld, const MediumInterface& mediumInterface,
			const Spectrum& I, const std::string& texName);
		Spectrum Sample_Li(const Interaction& ref, const Point2f& u, Vector3f* wi, float* pdf, VisibilityTester* vis) const override;
		Spectrum Scale(const Vector3f& w) const;
		Spectrum Power() const override;
	private:
		const Point3f pLight;
		const Spectrum I;
		std::unique_ptr<MIPMap<RGBSpectrum>> mipmap;
	};
}

#endif