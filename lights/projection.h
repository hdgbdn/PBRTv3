#ifndef PBRT_LIGHTS_PROJECTION_H
#define PBRT_LIGHTS_PROJECTION_H

#include "light.h"
#include "mipmap.h"
#include "imageio.h"

namespace pbrt
{
	class ProjectionLight : public Light
	{
	public:
		ProjectionLight(const Transform& LightToWorld, const MediumInterface& mediumInterface,
			const Spectrum& I, const std::string& texname, float fov);
		Spectrum Projection(const Vector3f& w) const;
		Spectrum Power() const override;
	private:
		std::unique_ptr<MIPMap<RGBSpectrum>> projectionMap;
		const Point3f pLight;
		const Spectrum I;
		Transform lightProjection;
		float near, far;
		Bounds2f screenBounds;
		float cosTotalWidth;
	};
}

#endif