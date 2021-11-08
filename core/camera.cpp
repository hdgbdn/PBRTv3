#include "camera.h"

namespace pbrt
{
	Camera::Camera(const AnimatedTransform& cameraToWorld, float shutterOpen, float shutterClose, Film* film,
	               const Medium* medium)
		: CameraToWorld(cameraToWorld),
		  shutterOpen(shutterOpen), shutterClose(shutterClose), film(film), medium(medium)
	{
	}

	float Camera::GenerateRayDifferential(const CameraSample& sample, RayDifferential* rd) const
	{
		float wt = GenerateRay(sample, rd);
		CameraSample sshift = sample;
		sshift.pFilm.x++;
		Ray rx;
		float wtx = GenerateRay(sshift, &rx);
		if (wtx == 0) return 0;
		rd->rxOrigin = rx.o;
		rd->rxDirection = rx.d;
		sshift.pFilm.y++;
		Ray ry;
		float wty = GenerateRay(sshift, &rx);
		if (wty == 0) return 0;
		rd->ryOrigin = ry.o;
		rd->ryDirection = ry.d;
		rd->hasDifferentials = true;
		return wt;
	}

	ProjectiveCamera::ProjectiveCamera(const AnimatedTransform& CameraToWorld, const Transform& CameraToScreen,
	                                   const Bounds2f& screenWindow, float shutterOpen, float shutterClose, float lensr,
	                                   float focald, Film* film, const Medium* medium)
		: Camera(CameraToWorld, shutterOpen, shutterClose, film, medium),
		  CameraToScreen(CameraToScreen), lensRadius(lensr), focalDistance(focald);
	{
		ScreenToRaster = Scale(film->fullResolution.x,
			film->fullResolution.y, 1) *
			Scale(1 / (screenWindow.pMax.x - screenWindow.pMin.x),
				1 / (screenWindow.pMin.y - screenWindow.pMax.y), 1) *
			Translate(Vector3f(-screenWindow.pMin.x, -screenWindow.pMax.y, 0));
		RasterToCamera = Inverse(CameraToScreen) * RasterToScreen;
	}

}