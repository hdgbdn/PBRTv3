#include "perspective.h"
#include "core/paramset.h"
#include "core/sampling.h"

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
		if (lensRadius > 0)
		{
			Point2f pLens = lensRadius * ConcentricSampleDisk(sample.pLens);
			float ft = focalDistance / ray->d.z;
			Point3f pFocus = (*ray)(ft);
			ray->o = Point3f(pLens.x, pLens.y, 0);
			ray->d = Normalize(pFocus - ray->o);
		}
		ray->time = Lerp(sample.time, shutterOpen, shutterClose);
		ray->medium = medium;
		*ray = CameraToWorld(*ray);
		return 1;
	}

	float PerspectiveCamera::GenerateRayDifferential(const CameraSample& sample, RayDifferential* rd) const
	{
		Point3f pFilm = Point3f(sample.pFilm.x, sample.pFilm.y, 0);
		Point3f pCamera = RasterToCamera(pFilm);
		Vector3f dir = Normalize(Vector3f(pCamera.x, pCamera.y, pCamera.z));
		*rd = RayDifferential(Point3f(0, 0, 0), dir);

		if (lensRadius > 0)
		{
			Point2f pLens = lensRadius * ConcentricSampleDisk(sample.pLens);
			float ft = focalDistance / rd->d.z;
			Point3f pFocus = (*rd)(ft);
			rd->o = Point3f(pLens.x, pLens.y, 0);
			rd->d = Normalize(pFocus - rd->o);
		}

		if (lensRadius > 0) 
		{
			Point2f pLens = lensRadius * ConcentricSampleDisk(sample.pLens);
			Vector3f dx = Normalize(Vector3f(pCamera + dxCamera));
			float ft = focalDistance / dx.z;
			Point3f pFocus = Point3f(0, 0, 0) + (ft * dx);
			rd->rxOrigin = Point3f(pLens.x, pLens.y, 0);
			rd->rxDirection = Normalize(pFocus - rd->rxOrigin);

			Vector3f dy = Normalize(Vector3f(pCamera + dyCamera));
			ft = focalDistance / dy.z;
			pFocus = Point3f(0, 0, 0) + (ft * dy);
			rd->ryOrigin = Point3f(pLens.x, pLens.y, 0);
			rd->ryDirection = Normalize(pFocus - rd->ryOrigin);
		}
		else 
		{
			rd->rxOrigin = rd->ryOrigin = rd->o;
			rd->rxDirection = Normalize(Vector3f(pCamera) + dxCamera);
			rd->ryDirection = Normalize(Vector3f(pCamera) + dyCamera);
		}
		rd->time = Lerp(sample.time, shutterOpen, shutterClose);
		rd->hasDifferentials = true;
		rd->medium = medium;
		*rd = CameraToWorld(*rd);
		return 1;
	}

	PerspectiveCamera* CreatePerspectiveCamera(const ParamSet& params, const AnimatedTransform& cam2world, Film* film,
		const Medium* medium)
	{
		float shutteropen = params.FindOneFloat("shutteropen", 0.f);
		float shutterclose = params.FindOneFloat("shutterclose", 1.f);
		if (shutterclose < shutteropen)
		{
			Warning("Shutter close time [%f] < shutter open [%f].  Swapping them.",
				shutterclose, shutteropen);
			std::swap(shutterclose, shutteropen);
		}
		float lensradius = params.FindOneFloat("lensradius", 0.f);
		float focaldistance = params.FindOneFloat("focaldistance", 1e6);
		float frame = params.FindOneFloat(
			"frameaspectratio",
			float(film->fullResolution.x) / float(film->fullResolution.y));
		Bounds2f screen;
		if (frame > 1.f) {
			screen.pMin.x = -frame;
			screen.pMax.x = frame;
			screen.pMin.y = -1.f;
			screen.pMax.y = 1.f;
		}
		else {
			screen.pMin.x = -1.f;
			screen.pMax.x = 1.f;
			screen.pMin.y = -1.f / frame;
			screen.pMax.y = 1.f / frame;
		}
		int swi;
		const float* sw = params.FindFloat("screenwindow", &swi);
		if (sw) {
			if (swi == 4) {
				screen.pMin.x = sw[0];
				screen.pMax.x = sw[1];
				screen.pMin.y = sw[2];
				screen.pMax.y = sw[3];
			}
			else
				Error("\"screenwindow\" should have four values");
		}
		float fov = params.FindOneFloat("fov", 90.);
		float halffov = params.FindOneFloat("halffov", -1.f);
		if (halffov > 0.f)
			// hack for structure synth, which exports half of the full fov
			fov = 2.f * halffov;
		return new PerspectiveCamera(cam2world, screen, shutteropen, shutterclose,
			lensradius, focaldistance, fov, film, medium);
	}
}
