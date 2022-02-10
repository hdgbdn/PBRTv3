#ifndef PBRT_CAMERA_ORTHO_H
#define PBRT_CAMERA_ORTHO_H

#include "core/pbrt.h"
#include "core/camera.h"

namespace pbrt
{
	class OrthographicCamera : public ProjectiveCamera
	{
		OrthographicCamera(const AnimatedTransform& CameraToWorld,
		                   const Bounds2f& screenWindow, float shutterOpen,
		                   float shutterClose, float lensr, float focald, Film* film, const Medium* medium);
		float GenerateRay(const CameraSample& sample, Ray* ray) const override;
		float GenerateRayDifferential(const CameraSample& sample, RayDifferential* rd) const override;
	private:
		Vector3f dxCamera, dyCamera;
	};
}

#endif