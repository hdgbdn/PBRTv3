#ifndef PBRT_CORE_SPECTRUM_H
#define PBRT_CORE_SPECTRUM_H

#include "pbrt.h"

namespace pbrt
{
	static const int nSpectralSamples = 60;
	enum class SpectrumType { Reflectance, Illuminant };
	template <int nSpectrumSamples>
	class CoefficientSpectrum
	{
	public:
		CoefficientSpectrum(float v = 0.f) {};
		CoefficientSpectrum& operator+=(const CoefficientSpectrum& s2){}
		bool IsBlack() const {}

		CoefficientSpectrum operator*(const CoefficientSpectrum& sp) const {
			// DCHECK(!sp.HasNaNs());
			CoefficientSpectrum ret = *this;
			for (int i = 0; i < nSpectrumSamples; ++i) ret.c[i] *= sp.c[i];
			return ret;
		}

		CoefficientSpectrum operator*(float a) const {
			CoefficientSpectrum ret = *this;
			for (int i = 0; i < nSpectrumSamples; ++i) ret.c[i] *= a;
			//DCHECK(!ret.HasNaNs());
			return ret;
		}

		CoefficientSpectrum operator/(float a) const {
			//CHECK_NE(a, 0);
			//DCHECK(!std::isnan(a));
			CoefficientSpectrum ret = *this;
			for (int i = 0; i < nSpectrumSamples; ++i) ret.c[i] /= a;
			//DCHECK(!ret.HasNaNs());
			return ret;
		}
	};

	class SampledSpectrum : public CoefficientSpectrum<nSpectralSamples>
	{
	public:
		SampledSpectrum(float v = 0.f) : CoefficientSpectrum(v) {}

		SampledSpectrum(const RGBSpectrum& r,
			SpectrumType type = SpectrumType::Reflectance);
	};

	class RGBSpectrum : public CoefficientSpectrum<3>
	{
	public:
		RGBSpectrum(float v = 0.f) : CoefficientSpectrum<3>(v) {}
		RGBSpectrum(const CoefficientSpectrum<3>& v) : CoefficientSpectrum<3>(v) {}
	};
}

#endif