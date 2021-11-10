#include "perspective.h"

namespace pbrt {
	PerspectiveCamera::PerspectiveCamera(const AnimatedTransform& CameraToWorld, const Bounds2f& screenWindow,
	                                     float shutterOpen, float shutterClose, float lensr, float focald, float fov,
	                                     Film* film, const Medium* medium)
		: ProjectiveCamera(CameraToWorld, Perspective(fov, 1e-2f, 1000.f),
		                   screenWindow, shutterOpen, shutterClose,
		                   lensr, focald, film, medium)
	{
		dxCamera = (RasterToCamera(Point3f(1, 0, 0)) -
			RasterToCamera(Point3f(0, 0, 0)));
		dyCamera = (RasterToCamera(Point3f(0, 1, 0)) -
			RasterToCamera(Point3f(0, 0, 0)));
	}

	float PerspectiveCamera::GenerateRay(const CameraSample& sample, Ray* ray) const
	{
		Point3f pFilm = Point3f(sample.pFilm.x, sample.pFilm.y, 0);
		Point3f pCamera = RasterToCamera(pFilm);
		*ray = Ray(Point3f(0, 0, 0), Normalize(Vector3f(pCamera)));
		// TODO Modify ray for depth of field
		ray->time = Lerp(sample.time, shutterOpen, shutterClose);
		ray->medium = medium;
		*ray = CameraToWorld(*ray);
		return 1;
	}

	float PerspectiveCamera::GenerateRayDifferential(const CameraSample& sample, RayDifferential* rd) const
	{
		Point3f pFilm = Point3f(sample.pFilm.x, sample.pFilm.y, 0);
		Point3f pCamera = RasterToCamera(pFilm);
		*rd = Ray(pCamera, Vector3f(0, 0, 1));

		// TODO Modify ray for depth of field

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