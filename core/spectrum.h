#ifndef PBRT_CORE_SPECTRUM_H
#define PBRT_CORE_SPECTRUM_H

#include "pbrt.h"

namespace pbrt
{
	extern bool SpectrumSamplesSorted(const float* lambda, const float* vals, int n);
	extern void SortSpectrumSamples(float* lambda, float* vals, int n);
	extern float AverageSpectrumSamples(const float* lambda, const float* vals,
	                                    int n, float lambdaStart, float lambdaEnd);
	static const int sampledLambdaStart = 400;
	static const int sampledLambdaEnd = 700;
	static const int nSpectralSamples = 60;

	enum class SpectrumType { Reflectance, Illuminant };
	template <int nSpectrumSamples>
	class CoefficientSpectrum
	{
	public:
		CoefficientSpectrum(float v = 0.f)
		{
			for (int i = 0; i < nSpectrumSamples; ++i)
				c[i] = v;
		}

		CoefficientSpectrum& operator+=(const CoefficientSpectrum& s2)
		{
			for (int i = 0; i < nSpectrumSamples; ++i)
				c[i] += s2.c[i];
			return *this;
		}

		CoefficientSpectrum operator+(const CoefficientSpectrum& s2) const
		{
			CoefficientSpectrum ret(*this);
			for (int i = 0; i < nSpectrumSamples; ++i)
				ret[i] += s2.c[i];
			return ret;
		}

		CoefficientSpectrum operator*(const CoefficientSpectrum& sp) const
		{
			// DCHECK(!sp.HasNaNs());
			CoefficientSpectrum ret = *this;
			for (int i = 0; i < nSpectrumSamples; ++i) ret.c[i] *= sp.c[i];
			return ret;
		}

		CoefficientSpectrum operator*(float a) const
		{
			CoefficientSpectrum ret = *this;
			for (int i = 0; i < nSpectrumSamples; ++i) ret.c[i] *= a;
			//DCHECK(!ret.HasNaNs());
			return ret;
		}

		CoefficientSpectrum operator/(float a) const
		{
			//CHECK_NE(a, 0);
			//DCHECK(!std::isnan(a));
			CoefficientSpectrum ret = *this;
			for (int i = 0; i < nSpectrumSamples; ++i) ret.c[i] /= a;
			//DCHECK(!ret.HasNaNs());
			return ret;
		}

		CoefficientSpectrum Clamp(float low = 0, float high = Infinity) const
		{
			CoefficientSpectrum ret = *this;
			for (int i = 0; i < nSpectrumSamples; ++i)
				ret.c[i] = Clamp(c[i], low, high);
			return ret;
		}

		friend CoefficientSpectrum Sqrt(const CoefficientSpectrum& s)
		{
			CoefficientSpectrum ret;
			for (int i = 0; i < nSpectrumSamples; ++i)
				ret.c[i] = std::sqrt(s.c[i]);
			return ret;
		}

		friend inline CoefficientSpectrum operator*(float a,
		                                            const CoefficientSpectrum& s)
		{
			//DCHECK(!std::isnan(a) && !s.HasNaNs());
			return s * a;
		}

		bool IsBlack() const
		{
			for (int i = 0; i < nSpectrumSamples; ++i)
				if (c[i] != 0) return false;
			return true;
		}

		bool HasNaNs() const
		{
			for (int i = 0; i < nSpectrumSamples; ++i)
				if (std::isnan(c[i])) return true;
			return false;
		}

		float& operator[](int i)
		{
			return c[i];
		}
	protected:
		float c[nSpectrumSamples];
	};

	class SampledSpectrum : public CoefficientSpectrum<nSpectralSamples>
	{
	public:
		SampledSpectrum(float v = 0.f) : CoefficientSpectrum(v) {}

		static SampledSpectrum FromSampled(const float* lambda,
		                                   const float* v, int n)
		{
			if(!SpectrumSamplesSorted(lambda, v, n))
			{
				std::vector<float> slambda(&lambda[0], &lambda[n]);
				std::vector<float> sv(&v[0], &v[n]);
				SortSpectrumSamples(&slambda[0], &sv[0], n);
				return FromSampled(&slambda[0], &sv[0], n);
			}
			SampledSpectrum r;
			for(int i = 0; i < nSpectralSamples; ++i)
			{
				float lambda0 = Lerp(float(i) / float(nSpectralSamples),
					sampledLambdaStart, sampledLambdaEnd);
				float lambda1 = Lerp(float(i + 1) / float(nSpectralSamples),
					sampledLambdaStart, sampledLambdaEnd);
				r.c[i] = AverageSpectrumSamples(lambda, v, n, lambda0, lambda1);
			}
			return r;
		}

		SampledSpectrum(const RGBSpectrum& r,
			SpectrumType type = SpectrumType::Reflectance);
	};

	class RGBSpectrum : public CoefficientSpectrum<3>
	{
	public:
		RGBSpectrum(float v = 0.f) : CoefficientSpectrum<3>(v) {}
		RGBSpectrum(const CoefficientSpectrum<3>& v) : CoefficientSpectrum<3>(v) {}
	};

	inline Spectrum Lerp(float t, const Spectrum& s1, const Spectrum& s2) {
		return (1 - t) * s1 + t * s2;
	}
}

#endif