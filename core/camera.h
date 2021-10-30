#ifndef PBRT_CORE_CAMERA_H
#define PBRT_CORE_CAMERA_H

#include "pbrt.h"
#include "film.h"
#include "transformation.h"

namespace pbrt
{
	class Camera
	{
	public:
		Camera(const AnimatedTransform& cameraToWorld, float shutterOpen,
			float shutterClose, Film* film, const Medium* medium);
		virtual float GenerateRay(const CameraSample& sample, Ray* ray) const = 0;
		virtual float GenerateRayDifferential(const CameraSample& sample,
			RayDifferential* rd) const;

		AnimatedTransform CameraToWorld;
		const float shutterOpen, shutterClose;
		Film* film;
		const Medium* medium;
	};

	struct CameraSample {
		Point2f pFilm;
		Point2f pLens;
		float time;
	};

	class ProjectiveCamera :public Camera
	{
	public:
		ProjectiveCamera(const AnimatedTransform& CameraToWorld, const Transform& CameraToScreen,
		                 const Bounds2f& screenWindow, float shutterOpen,
		                 float shutterClose, float lensr, float focald, Film* film, const Medium* medium);
	protected:
		Transform CameraToScreen, RasterToCamera;
		Transform ScreenToRaster, RasterToScreen;
	};

}

#endif