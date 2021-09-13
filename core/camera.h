#ifndef PBRT_CORE_CAMERA_H
#define PBRT_CORE_CAMERA_H

#include "pbrt.h"
#include "film.h"

namespace pbrt
{
	class Camera
	{
	public:
		virtual float GenerateRay(const CameraSample& sample, Ray* ray) const = 0;
		virtual float GenerateRayDifferential(const CameraSample& sample,
			RayDifferential* rd) const;
		std::shared_ptr<Film> film;
	};

	struct CameraSample {
		Point2f pFilm;
		Point2f pLens;
		float time;
	};
}

#endif