#include "sampler.h"

namespace pbrt
{
	void Sampler::StartPixel(const Point2i& p)
	{
		currentPixel = p;
		currentPixelSampleIndex = 0;
		array1DOffset = array2DOffset = 0;
	}

	bool Sampler::StartNextSample()
	{
		array1DOffset = array2DOffset = 0;
		return ++currentPixelSampleIndex < samplesPerPixel;
	}

	bool Sampler::SetSampleNumber(int64_t sampleNum)
	{
		array1DOffset = array2DOffset = 0;
		currentPixelSampleIndex = sampleNum;
		return currentPixelSampleIndex < samplesPerPixel;
	}

	void Sampler::Request1DArray(int n)
	{
		samples1DArraySizes.push_back(n);
		sampleArray1D.push_back(std::vector<float>(n * samplesPerPixel));
	}

	void Sampler::Request2DArray(int n)
	{
		samples2DArraySizes.push_back(n);
		sampleArray2D.push_back(std::vector<Point2f>(n * samplesPerPixel));
	}

	const float* Sampler::Get1DArray(int n)
	{
		if (array1DOffset == sampleArray1D.size())
			return nullptr;
		return &sampleArray1D[array1DOffset++][currentPixelSampleIndex * n];
	}

	const Point2f* Sampler::Get2DArray(int n)
	{
		if (array2DOffset == sampleArray2D.size())
			return nullptr;
		return &sampleArray2D[array2DOffset++][currentPixelSampleIndex * n];
	}



}