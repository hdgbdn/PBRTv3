#include "orthographic.h"

namespace pbrt
{
	OrthographicCamera::OrthographicCamera(const AnimatedTransform& CameraToWorld, const Bounds2f& screenWindow,
	                                       float shutterOpen, float shutterClose, float lensr, float focald, Film* film,
	                                       const Medium* medium)
		: ProjectiveCamera(CameraToWorld, Orthographic(0, 1),
		                   screenWindow, shutterOpen, shutterClose,
		                   lensr, focald, film, medium)
	{
		dxCamera = RasterToCamera(Vector3f(1, 0, 0));
		dyCamera = RasterToCamera(Vector3f(0, 1, 0));
	}

	float OrthographicCamera::GenerateRay(const CameraSample& sample, Ray* ray) const
	{
		Point3f pFilm = Point3f(sample.pFilm.x, sample.pFilm.y, 0);
		Point3f pCamera = RasterToCamera(pFilm);
		*ray = Ray(pCamera, Vector3f(0, 0, 1));
		// TODO DOF
		ray->time = Lerp(sample.time, shutterOpen, shutterClose);
		ray->medium = medium;
		*ray = CameraToWorld(*ray);
		return 1;
	}

	float OrthographicCamera::GenerateRayDifferential(const CameraSample& sample, RayDifferential* rd) const
	{
		Point3f pFilm = Point3f(sample.pFilm.x, sample.pFilm.y, 0);
		Point3f pCamera = RasterToCamera(pFilm);
		*rd = Ray(pCamera, Vector3f(0, 0, 1));

		if (lensRadius > 0) {
			//TODO Compute OrthographicCamera ray differentials accounting for lens >>
		}
		else {
			rd->rxOrigin = rd->o + dxCamera;
			rd->ryOrigin = rd->o + dyCamera;
			rd->rxDirection = rd->ryDirection = rd->d;
		}
		rd->time = Lerp(sample.time, shutterOpen, shutterClose);
		rd->hasDifferentials = true;
		rd->medium = medium;
		*rd = CameraToWorld(*rd);
		return 1;
	}

}
