#ifndef PBRT_CORE_PBRT_H
#define PBRT_CORE_PBRT_H

#include <iostream>
#include <fmt/core.h>
#include <string>
#include <limits>
#include <vector>
#include <memory>

namespace pbrt
{
#ifdef _MSC_VER
	constexpr float MaxFloat = std::numeric_limits<float>::max();
	constexpr float Infinity = std::numeric_limits<float>::infinity();
#else
	static PBRT_CONSTEXPR Float MaxFloat = std::numeric_limits<Float>::max();
	static PBRT_CONSTEXPR Float Infinity = std::numeric_limits<Float>::infinity();
#endif

	struct Options {};

	// Forward
	class Scene;
	class Ray;
	class RayDifferential;
	class SurfaceInteraction;
	class Integrator;
	class SamplerIntegrator;
	class Sampler;
	class Camera;
	struct CameraSample;
	class Film;
	class FilmTile;
	class MemoryArena;
	class RGBSpectrum;
	class SampledSpectrum;
#ifdef PBRT_SAMPLED_SPECTRUM
	typedef SampledSpectrum Spectrum;
#else
	typedef RGBSpectrum Spectrum;
#endif
	class Light;
	class VisibilityTester;
	struct Interaction;
	class SurfaceInteraction;
	class Medium;

	// geometry
	template <typename T>
	class Vector2;
	template <typename T>
	class Vector3;
	template <typename T>
	class Point3;
	template <typename T>
	class Point2;
	template <typename T>
	class Normal3;
	class Ray;
	class RayDifferential;
	template <typename T>
	class Bounds2;
}

#endif