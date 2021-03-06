#ifndef PBRT_CAMERA_PERSPECTIVE_H
#define PBRT_CAMERA_PERSPECTIVE_H

#include "core/pbrt.h"
#include "core/camera.h"

namespace pbrt
{
	class PerspectiveCamera : public ProjectiveCamera
	{
	public:
		PerspectiveCamera(const AnimatedTransform& CameraToWorld,
		                  const Bounds2f& screenWindow, float shutterOpen,
		                  float shutterClose, float lensr, float focald,
		                  float fov, Film* film, const Medium* medium);
		float GenerateRay(const CameraSample& sample, Ray* ray) const override;
		float GenerateRayDifferential(const CameraSample& sample, RayDifferential* rd) const override;
	private:
		Vector3f dxCamera, dyCamera;
		float A;
	};

	PerspectiveCamera* CreatePerspectiveCamera(const ParamSet& params,
		const AnimatedTransform& cam2world,
		Film* film, const Medium* medium);
}

#endif