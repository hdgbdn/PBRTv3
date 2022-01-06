#include "sampling.h"

namespace pbrt
{
	void StratifiedSample1D(float* samples, int nSamples, RNG& rng, bool jitter)
	{
		float invNSamples = (float)1 / nSamples;
		for (int i = 0; i < nSamples; ++i)
		{
			float delta = jitter ? rng.UniformFloat() : 0.5f;
			samples[i] = std::min((i + delta) * invNSamples, OneMinusEpsilon);
		}
	}

	void StratifiedSample2D(Point2f* samp, int nx, int ny, RNG& rng,
	                        bool jitter)
	{
		float dx = (float)1 / nx, dy = (float)1 / ny;
		for (int y = 0; y < ny; ++y)
			for (int x = 0; x < nx; ++x)
			{
				float jx = jitter ? rng.UniformFloat() : 0.5f;
				float jy = jitter ? rng.UniformFloat() : 0.5f;
				samp->x = std::min((x + jx) * dx, OneMinusEpsilon);
				samp->y = std::min((y + jy) * dy, OneMinusEpsilon);
				++samp;
			}
	}

	void LatinHypercube(float* samples, int nSamples, int nDim, RNG& rng)
	{
		float invNSamples = (float)1 / nSamples;
		for (int i = 0; i < nSamples; ++i)
			for (int j = 0; j < nDim; ++j)
			{
				float sj = (i + (rng.UniformFloat())) * invNSamples;
				samples[nDim * i + j] = std::min(sj, OneMinusEpsilon);
			}
		for (int i = 0; i < nDim; ++i) {
			for (int j = 0; j < nSamples; ++j) {
				int other = j + rng.UniformUInt32(nSamples - j);
				std::swap(samples[nDim * j + i], samples[nDim * other + i]);
			}
		}
	}

	Point2f RejectionSampleDisk(RNG& rng)
	{
		Point2f p;
		do
		{
			p.x = 1 - 2 * rng.UniformFloat();
			p.y = 1 - 2 * rng.UniformFloat();
		} while (p.x * p.x + p.y * p.y > 1);
		return p;
	}

	Vector3f UniformSampleHemisphere(const Point2f& u)
	{
		float z = u[0];
		float r = std::sqrt(std::max((float)0, (float)1. - z * z));
		float phi = 2 * Pi * u[1];
		return {r * std::cos(phi), r * std::sin(phi), z};
	}

	Vector3f UniformSampleSphere(const Point2f& u)
	{
		float z = 1 - 2 * u[0];
		float r = std::sqrt(std::max((float)0, (float)1. - z * z));
		float phi = 2 * Pi * u[1];
		return {r * std::cos(phi), r * std::sin(phi), z};
	}

	Point2f UniformSampleDisk(const Point2f& u)
	{
		float r = std::sqrt(u[0]);
		float theta = 2 * Pi * u[1];
		return { r * std::cos(theta), r * std::sin(theta) };
	}

	Point2f ConcentricSampleDisk(const Point2f& u)
	{
		Point2f uOffset = 2.f * u - Vector2f(1, 1);
		if (uOffset.x == 0 && uOffset.y == 0) return {0, 0};
		float theta, r;
		if (std::abs(uOffset.x) > std::abs(uOffset.y))
		{
			r = uOffset.x;
			theta = PiOver4 * (uOffset.y / uOffset.x);
		}
		else
		{
			r = uOffset.y;
			theta = PiOver2 - PiOver4 * (uOffset.x / uOffset.y);
		}
		return r * Point2f(std::cos(theta), std::sin(theta));
	}

	Vector3f UniformSampleCone(const Point2f& u, float cosThetaMax)
	{
		float cosTheta = ((float)1 - u[0]) + u[0] * cosThetaMax;
		float sinTheta = std::sqrt((float)1 - cosTheta * cosTheta);
		float phi = u[1] * 2 * Pi;
		return { std::cos(phi) * sinTheta, std::sin(phi) * sinTheta,cosTheta };
	}
}
