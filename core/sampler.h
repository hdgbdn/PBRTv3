#ifndef PBRT_CORE_SAMPLER_H
#define PBRT_CORE_SAMPLER_H

#include "camera.h"
#include "pbrt.h"
#include "geometry.h"
#include "rng.h"

namespace pbrt
{
	class Sampler
	{
	public:
		Sampler(int64_t samplesPerPixel);
		virtual std::unique_ptr<Sampler> Clone(int seed) = 0;
		virtual void StartPixel(const Point2i& p);
		virtual bool StartNextSample();
		CameraSample GetCameraSample(const Point2i& pRaster)
		{
			CameraSample cs;
			cs.pFilm = static_cast<Point2f>(pRaster) + Get2D();
			cs.time = Get1D();
			cs.pLens = Get2D();
			return cs;
		}
		virtual bool SetSampleNumber(int64_t sampleNum);
		virtual float Get1D() = 0;
		virtual Point2f Get2D() = 0;
		void Request1DArray(int n);
		void Request2DArray(int n);
		const float* Get1DArray(int n);
		const Point2f* Get2DArray(int n);
		virtual int RoundCount(int n) const
		{
			return n;
		}
		const int64_t samplesPerPixel;
	protected:
		Point2i currentPixel;
		int64_t currentPixelSampleIndex;
		std::vector<int> samples1DArraySizes, samples2DArraySizes;
		std::vector<std::vector<float>> sampleArray1D;
		std::vector<std::vector<Point2f>> sampleArray2D;
	private:
		size_t array1DOffset, array2DOffset;
	};

	class PixelSampler : public Sampler
	{
	public:
		PixelSampler(int64_t samplesPerPixel, int nSampledDimensions)
			: Sampler(samplesPerPixel)
		{
			for (int i = 0; i < nSampledDimensions; ++i)
			{
				samples1D.push_back(std::vector<float>(samplesPerPixel));
				samples2D.push_back(std::vector<Point2f>(samplesPerPixel));
			}
		}

		bool StartNextSample() override
		{
			current1DDimension = current2DDimension = 0;
			return Sampler::StartNextSample();
		}

		bool SetSampleNumber(int64_t sampleNum) override
		{
			current1DDimension = current2DDimension = 0;
			return Sampler::SetSampleNumber(sampleNum);
		}

		float Get1D() override
		{
			if (current1DDimension < samples1D.size())
				return samples1D[current1DDimension++][currentPixelSampleIndex];
			else
				return rng.UniformFloat();
		}

		Point2f Get2D() override
		{
			if (current2DDimension < samples2D.size())
				return samples2D[current2DDimension++][currentPixelSampleIndex];
			else
				return Point2f(rng.UniformFloat(), rng.UniformFloat());
		}

	protected:
		std::vector<std::vector<float>> samples1D;
		std::vector<std::vector<Point2f>> samples2D;
		int current1DDimension = 0, current2DDimension = 0;
		RNG rng;
	};
}

#endif