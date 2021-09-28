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
	static constexpr float MaxFloat = std::numeric_limits<float>::max();
	static constexpr float Infinity = std::numeric_limits<float>::infinity();
	static constexpr float Pi = 3.14159265358979323846f;
#else
	static PBRT_CONSTEXPR Float MaxFloat = std::numeric_limits<Float>::max();
	static PBRT_CONSTEXPR Float Infinity = std::numeric_limits<Float>::infinity();
#endif

	struct Options {};

	// Forward
	class Transform;
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
	class MediumInteraction;
	class Medium;
	class MediumInterface;
	class EFloat;

	template <typename T>
	class Texture;

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

	// shapes
	class Shape;

	inline float Radians(float deg) { return (Pi / 180) * deg; }

	inline float Degrees(float rad) { return (180 / Pi) * rad; }
	template <typename T, typename U, typename V>
	inline T Clamp(T val, U low, V high) {
		if (val < low)
			return low;
		else if (val > high)
			return high;
		else
			return val;
	}
	inline bool Quadratic(float a, float b, float c, float* t0, float* t1);
}

#endif