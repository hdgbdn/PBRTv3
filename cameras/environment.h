#ifndef PBRT_CAMERA_ENVIRONMENT_H
#define PBRT_CAMERA_ENVIRONMENT_H

#include "core/pbrt.h"
#include "core/camera.h"

namespace pbrt
{
	class EnvironmentCamera : public Camera
	{
	public:
		EnvironmentCamera(const AnimatedTransform& CameraToWorld,
		                  float shutterOpen, float shutterClose, Film* film,
		                  const Medium* medium)
		: Camera(CameraToWorld, shutterOpen, shutterClose, film, medium) {}
		float GenerateRay(const CameraSample& sample, Ray* ray) const override;
		float GenerateRayDifferential(const CameraSample& sample, RayDifferential* rd) const override;
	private:

	};
}

#endif