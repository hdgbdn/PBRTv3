#ifndef PBRT_CORE_PBRT_H
#define PBRT_CORE_PBRT_H

#include <iostream>
#include <fmt/core.h>
#include <string>
#include <limits>
#include <vector>
#include <memory>
#include <algorithm>

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
	class AnimatedTransform;
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
	class Primitive;
	class Light;
	class AreaLight;
	class Material;
	class VisibilityTester;
	struct Interaction;
	class SurfaceInteraction;
	class MediumInteraction;
	class Medium;
	class MediumInterface;
	class BSDF;
	class BSSRDF;
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

	inline float Log2(float x) {
		const float invLog2 = 1.442695040888963387004650940071;
		return std::log(x) * invLog2;
	}

	inline int Log2Int(uint32_t v) {
		unsigned long lz = 0;
		if (_BitScanReverse(&lz, v)) return lz;
		return 0;
	}

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
	inline float Lerp(float t, float v1, float v2) { return (1 - t) * v1 + t * v2; }

}

#endif