#ifndef PBRT_CORE_PBRT_H
#define PBRT_CORE_PBRT_H

#include <iostream>
#include <fmt/core.h>
#include <string>
#include <limits>
#include <vector>
#include <memory>
#include <algorithm>
#include <mutex>

namespace pbrt
{
#ifdef _MSC_VER
	static constexpr float MaxFloat = std::numeric_limits<float>::max();
	static constexpr float Infinity = std::numeric_limits<float>::infinity();
#else
	static PBRT_CONSTEXPR Float MaxFloat = std::numeric_limits<Float>::max();
	static PBRT_CONSTEXPR Float Infinity = std::numeric_limits<Float>::infinity();
#endif
#ifndef PBRT_L1_CACHE_LINE_SIZE
#define PBRT_L1_CACHE_LINE_SIZE 64
#endif
#define ALLOCA(TYPE, COUNT) (TYPE *) alloca((COUNT) * sizeof(TYPE))
	static constexpr float MachineEpsilon =
		std::numeric_limits<float>::epsilon() * 0.5;

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
    class Normal2;
	template <typename T>
	class Normal3;
	class Ray;
	class RayDifferential;
	template <typename T>
	class Bounds2;

	// shapes
	class Shape;

	// samplers
	class RNG;

	// BxDF
	class MicrofacetDistribution;

	// material
	class MixMaterial;

	template <typename T>
	inline T Mod(T a, T b)
	{
		T result = a - (a / b) * b;
		return (T)((result < 0) ? result + b : result);
	}

	template <>
	inline float Mod(float a, float b)
	{
		return std::fmod(a, b);
	}

	// Global constants

	static constexpr float ShadowEpsilon = 0.0001f;
	static constexpr float Pi = 3.14159265358979323846;
	static constexpr float InvPi = 0.31830988618379067154;
	static constexpr float Inv2Pi = 0.15915494309189533577;
	static constexpr float Inv4Pi = 0.07957747154594766788;
	static constexpr float PiOver2 = 1.57079632679489661923;
	static constexpr float PiOver4 = 0.78539816339744830961;
	static constexpr float Sqrt2 = 1.41421356237309504880;

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
	template <typename Predicate>
	int FindInterval(int size, const Predicate& pred) {
		int first = 0, len = size;
		while (len > 0) {
			int half = len >> 1, middle = first + half;
			if (pred(middle)) {
				first = middle + 1;
				len -= half + 1;
			}
			else
				len = half;
		}
		return Clamp(first - 1, 0, size - 2);
	}

	inline uint32_t FloatToBits(float f) {
		uint32_t ui;
		memcpy(&ui, &f, sizeof(float));
		return ui;
	}

	inline float BitsToFloat(uint32_t ui) {
		float f;
		memcpy(&f, &ui, sizeof(uint32_t));
		return f;
	}

	inline uint64_t FloatToBits(double f) {
		uint64_t ui;
		memcpy(&ui, &f, sizeof(double));
		return ui;
	}

	inline double BitsToFloat(uint64_t ui) {
		double f;
		memcpy(&f, &ui, sizeof(uint64_t));
		return f;
	}

	inline float NextFloatUp(float v) {
		// Handle infinity and negative zero for _NextFloatUp()_
		if (std::isinf(v) && v > 0.) return v;
		if (v == -0.f) v = 0.f;

		// Advance _v_ to next higher float
		uint32_t ui = FloatToBits(v);
		if (v >= 0)
			++ui;
		else
			--ui;
		return BitsToFloat(ui);
	}

	inline float NextFloatDown(float v) {
		// Handle infinity and positive zero for _NextFloatDown()_
		if (std::isinf(v) && v < 0.) return v;
		if (v == 0.f) v = -0.f;
		uint32_t ui = FloatToBits(v);
		if (v > 0)
			--ui;
		else
			++ui;
		return BitsToFloat(ui);
	}

	inline double NextFloatUp(double v, int delta = 1) {
		if (std::isinf(v) && v > 0.) return v;
		if (v == -0.f) v = 0.f;
		uint64_t ui = FloatToBits(v);
		if (v >= 0.)
			ui += delta;
		else
			ui -= delta;
		return BitsToFloat(ui);
	}

	inline double NextFloatDown(double v, int delta = 1) {
		if (std::isinf(v) && v < 0.) return v;
		if (v == 0.f) v = -0.f;
		uint64_t ui = FloatToBits(v);
		if (v > 0.)
			ui -= delta;
		else
			ui += delta;
		return BitsToFloat(ui);
	}

	template <typename T> inline bool IsPowerOf2(T v) {
		return v && !(v & (v - 1));
	}

	inline int32_t RoundUpPow2(int32_t v) {
		v--;
		v |= v >> 1;    v |= v >> 2;
		v |= v >> 4;    v |= v >> 8;
		v |= v >> 16;
		return v + 1;
	}

	inline constexpr float gamma(int n) {
		return (n * MachineEpsilon) / (1 - n * MachineEpsilon);
	}

	inline float GammaCorrect(float value) {
		if (value <= 0.0031308f)
			return 12.92f * value;
		return 1.055f * std::pow(value, (float)(1.f / 2.4f)) - 0.055f;
	}

	inline float InverseGammaCorrect(float value) {
		if (value <= 0.04045f)
			return value * 1.f / 12.92f;
		return std::pow((value + 0.055f) * 1.f / 1.055f, (float)2.4f);
	}
}

#endif