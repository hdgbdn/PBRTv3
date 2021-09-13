#ifndef PBRT_CORE_SPECTRUM_H
#define PBRT_CORE_SPECTRUM_H

#include "pbrt.h"

namespace pbrt
{
	static const int nSpectralSamples = 60;

	template <int nSpectrumSamples>
	class CoefficientSpectrum {};

	class SampledSpectrum : public CoefficientSpectrum<nSpectralSamples>
	{
	public:
		SampledSpectrum(float v = 0.f);
	};

	class RGBSpectrum : public CoefficientSpectrum<3>
	{
	public:
		RGBSpectrum(float v = 0.f);
	};
}

#endif