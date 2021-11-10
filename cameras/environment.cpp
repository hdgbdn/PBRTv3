#include "environment.h"

namespace pbrt
{
	float EnvironmentCamera::GenerateRay(const CameraSample& sample, Ray* ray) const
	{
		float theta = Pi * sample.pFilm.y / film->fullResolution.y;
		float phi = 2 * Pi * sample.pFilm.x / film->fullResolution.x;
		Vector3f dir(std::sin(theta) * std::cos(phi), std::cos(theta),
			std::sin(theta) * std::sin(phi));
		*ray = Ray(Point3f(0, 0, 0), dir, Infinity,
			Lerp(sample.time, shutterOpen, shutterClose));
		ray->medium = medium;
		*ray = CameraToWorld(*ray);
		return 1;
	}

}