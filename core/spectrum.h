#ifndef PBRT_CORE_SPECTRUM_H
#define PBRT_CORE_SPECTRUM_H

#include "pbrt.h"

namespace pbrt
{
	extern bool SpectrumSamplesSorted(const float* lambda, const float* vals, int n);
	extern void SortSpectrumSamples(float* lambda, float* vals, int n);
	extern float AverageSpectrumSamples(const float* lambda, const float* vals,
	                                    int n, float lambdaStart, float lambdaEnd);
	extern float InterpolateSpectrumSamples(const float* lambda, const float* vals,
		int n, float l);
	static const int sampledLambdaStart = 400;
	static const int sampledLambdaEnd = 700;
	static const int nSpectralSamples = 60;

	static const int nCIESamples = 471;
	extern const float CIE_X[nCIESamples];
	extern const float CIE_Y[nCIESamples];
	extern const float CIE_Z[nCIESamples];
	extern const float CIE_lambda[nCIESamples];
	static const float CIE_Y_integral = 106.856895;

	static const int nRGB2SpectSamples = 32;
	extern const float RGB2SpectLambda[nRGB2SpectSamples];
	extern const float RGBRefl2SpectWhite[nRGB2SpectSamples];
	extern const float RGBRefl2SpectCyan[nRGB2SpectSamples];
	extern const float RGBRefl2SpectMagenta[nRGB2SpectSamples];
	extern const float RGBRefl2SpectYellow[nRGB2SpectSamples];
	extern const float RGBRefl2SpectRed[nRGB2SpectSamples];
	extern const float RGBRefl2SpectGreen[nRGB2SpectSamples];
	extern const float RGBRefl2SpectBlue[nRGB2SpectSamples];


	extern const float RGBIllum2SpectWhite[nRGB2SpectSamples];
	extern const float RGBIllum2SpectCyan[nRGB2SpectSamples];
	extern const float RGBIllum2SpectMagenta[nRGB2SpectSamples];
	extern const float RGBIllum2SpectYellow[nRGB2SpectSamples];
	extern const float RGBIllum2SpectRed[nRGB2SpectSamples];
	extern const float RGBIllum2SpectGreen[nRGB2SpectSamples];
	extern const float RGBIllum2SpectBlue[nRGB2SpectSamples];

	inline void XYZToRGB(const float xyz[3], float rgb[3]) {
		rgb[0] = 3.240479f * xyz[0] - 1.537150f * xyz[1] - 0.498535f * xyz[2];
		rgb[1] = -0.969256f * xyz[0] + 1.875991f * xyz[1] + 0.041556f * xyz[2];
		rgb[2] = 0.055648f * xyz[0] - 0.204043f * xyz[1] + 1.057311f * xyz[2];
	}

	inline void RGBToXYZ(const float rgb[3], float xyz[3]) {
		xyz[0] = 0.412453f * rgb[0] + 0.357580f * rgb[1] + 0.180423f * rgb[2];
		xyz[1] = 0.212671f * rgb[0] + 0.715160f * rgb[1] + 0.072169f * rgb[2];
		xyz[2] = 0.019334f * rgb[0] + 0.119193f * rgb[1] + 0.950227f * rgb[2];
	}

	enum class SpectrumType { Reflectance, Illuminant };
	template <int nSpectrumSamples>
	class CoefficientSpectrum
	{
	public:
		CoefficientSpectrum(float v = 0.f);

		CoefficientSpectrum& operator+=(const CoefficientSpectrum& s2);

		CoefficientSpectrum operator+(const CoefficientSpectrum& s2) const;

		CoefficientSpectrum operator-(const CoefficientSpectrum& s2) const;

		CoefficientSpectrum operator/(const CoefficientSpectrum& s2) const;

		CoefficientSpectrum operator*(const CoefficientSpectrum& sp) const;

		CoefficientSpectrum& operator*=(const CoefficientSpectrum& sp);


		CoefficientSpectrum Clamp(float low = 0, float high = Infinity) const;

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

		CoefficientSpectrum operator*(float a) const;

		CoefficientSpectrum& operator*=(float a);

		CoefficientSpectrum operator/(float a) const;

		CoefficientSpectrum& operator/=(float a);

		bool operator==(const CoefficientSpectrum& sp) const;

		bool operator!=(const CoefficientSpectrum& sp) const;

		bool IsBlack() const;

		bool HasNaNs() const;

		float& operator[](int i);

		float operator[](int i) const;

		static const int nSamples = nSpectrumSamples;
	protected:
		float c[nSpectrumSamples];
	};

	template <int nSpectrumSamples>
	CoefficientSpectrum<nSpectrumSamples>::CoefficientSpectrum(float v)
	{
		for (int i = 0; i < nSpectrumSamples; ++i)
			c[i] = v;
	}

	template <int nSpectrumSamples>
	CoefficientSpectrum<nSpectrumSamples>& CoefficientSpectrum<nSpectrumSamples>::operator+=(
		const CoefficientSpectrum& s2)
	{
		//DCHECK(!s2.HasNaNs());
		for (int i = 0; i < nSpectrumSamples; ++i) c[i] += s2.c[i];
		return *this;
	}

	template <int nSpectrumSamples>
	CoefficientSpectrum<nSpectrumSamples> CoefficientSpectrum<nSpectrumSamples>::operator+(
		const CoefficientSpectrum& s2) const
	{
		//DCHECK(!s2.HasNaNs());
		CoefficientSpectrum ret = *this;
		for (int i = 0; i < nSpectrumSamples; ++i) ret.c[i] += s2.c[i];
		return ret;
	}

	template <int nSpectrumSamples>
	CoefficientSpectrum<nSpectrumSamples> CoefficientSpectrum<nSpectrumSamples>::operator-(
		const CoefficientSpectrum& s2) const
	{
		//DCHECK(!s2.HasNaNs());
		CoefficientSpectrum ret = *this;
		for (int i = 0; i < nSpectrumSamples; ++i) ret.c[i] -= s2.c[i];
		return ret;
	}

	template <int nSpectrumSamples>
	CoefficientSpectrum<nSpectrumSamples> CoefficientSpectrum<nSpectrumSamples>::operator/(
		const CoefficientSpectrum& s2) const
	{
		//DCHECK(!s2.HasNaNs());
		CoefficientSpectrum ret = *this;
		for (int i = 0; i < nSpectrumSamples; ++i) {
			//CHECK_NE(s2.c[i], 0);
			ret.c[i] /= s2.c[i];
		}
		return ret;
	}

	template <int nSpectrumSamples>
	CoefficientSpectrum<nSpectrumSamples> CoefficientSpectrum<nSpectrumSamples>::operator*(
		const CoefficientSpectrum& sp) const
	{
		//DCHECK(!sp.HasNaNs());
		CoefficientSpectrum ret = *this;
		for (int i = 0; i < nSpectrumSamples; ++i) ret.c[i] *= sp.c[i];
		return ret;
	}

	template <int nSpectrumSamples>
	CoefficientSpectrum<nSpectrumSamples>& CoefficientSpectrum<nSpectrumSamples>::operator*=(
		const CoefficientSpectrum& sp)
	{
		//DCHECK(!sp.HasNaNs());
		for (int i = 0; i < nSpectrumSamples; ++i) c[i] *= sp.c[i];
		return *this;
	}

	template <int nSpectrumSamples>
	CoefficientSpectrum<nSpectrumSamples> CoefficientSpectrum<nSpectrumSamples>::Clamp(float low, float high) const
	{
		CoefficientSpectrum ret;
		for (int i = 0; i < nSpectrumSamples; ++i)
			ret.c[i] = pbrt::Clamp(c[i], low, high);
		return ret;
	}

	template <int nSpectrumSamples>
	CoefficientSpectrum<nSpectrumSamples> CoefficientSpectrum<nSpectrumSamples>::operator*(float a) const
	{
		CoefficientSpectrum ret = *this;
		for (int i = 0; i < nSpectrumSamples; ++i) ret.c[i] *= a;
		//DCHECK(!ret.HasNaNs());
		return ret;
	}

	template <int nSpectrumSamples>
	CoefficientSpectrum<nSpectrumSamples>& CoefficientSpectrum<nSpectrumSamples>::operator*=(float a)
	{
		for (int i = 0; i < nSpectrumSamples; ++i) c[i] *= a;
		//DCHECK(!HasNaNs());
		return *this;
	}

	template <int nSpectrumSamples>
	CoefficientSpectrum<nSpectrumSamples> CoefficientSpectrum<nSpectrumSamples>::operator/(float a) const
	{
		//CHECK_NE(a, 0);
		//DCHECK(!std::isnan(a));
		CoefficientSpectrum ret = *this;
		for (int i = 0; i < nSpectrumSamples; ++i) ret.c[i] /= a;
		//DCHECK(!ret.HasNaNs());
		return ret;
	}

	template <int nSpectrumSamples>
	CoefficientSpectrum<nSpectrumSamples>& CoefficientSpectrum<nSpectrumSamples>::operator/=(float a)
	{
		//CHECK_NE(a, 0);
		//DCHECK(!std::isnan(a));
		for (int i = 0; i < nSpectrumSamples; ++i) c[i] /= a;
		return *this;
	}

	template <int nSpectrumSamples>
	bool CoefficientSpectrum<nSpectrumSamples>::operator==(const CoefficientSpectrum& sp) const
	{
		for (int i = 0; i < nSpectrumSamples; ++i)
			if (c[i] != sp.c[i]) return false;
		return true;
	}

	template <int nSpectrumSamples>
	bool CoefficientSpectrum<nSpectrumSamples>::operator!=(const CoefficientSpectrum& sp) const
	{
		return !(*this == sp);
	}

	template <int nSpectrumSamples>
	bool CoefficientSpectrum<nSpectrumSamples>::IsBlack() const
	{
		for (int i = 0; i < nSpectrumSamples; ++i)
			if (c[i] != 0) return false;
		return true;
	}

	template <int nSpectrumSamples>
	bool CoefficientSpectrum<nSpectrumSamples>::HasNaNs() const
	{
		for (int i = 0; i < nSpectrumSamples; ++i)
			if (std::isnan(c[i])) return true;
		return false;
	}

	template <int nSpectrumSamples>
	float& CoefficientSpectrum<nSpectrumSamples>::operator[](int i)
	{
		return c[i];
	}

	template <int nSpectrumSamples>
	float CoefficientSpectrum<nSpectrumSamples>::operator[](int i) const
	{
		// DCHECK(i >= 0 && i < nSpectrumSamples);
		return c[i];
	}

	class SampledSpectrum : public CoefficientSpectrum<nSpectralSamples>
	{
	public:
		SampledSpectrum(float v = 0.f);

		SampledSpectrum(const CoefficientSpectrum<nSpectralSamples>& v);

		static SampledSpectrum FromSampled(const float* lambda,
		                                   const float* v, int n);

		SampledSpectrum(const RGBSpectrum& r,
		                SpectrumType type = SpectrumType::Reflectance);

		static void Init();
		void ToXYZ(float xyz[3]) const;;
		float y() const;

		void ToRGB(float rgb[3]) const;

		RGBSpectrum ToRGBSpectrum() const;
		SampledSpectrum FromRGB(const float rgb[3],
			SpectrumType type);
	private:
		static SampledSpectrum X, Y, Z;
		static SampledSpectrum rgbRefl2SpectWhite, rgbRefl2SpectCyan;
		static SampledSpectrum rgbRefl2SpectMagenta, rgbRefl2SpectYellow;
		static SampledSpectrum rgbRefl2SpectRed, rgbRefl2SpectGreen;
		static SampledSpectrum rgbRefl2SpectBlue;

		static SampledSpectrum rgbIllum2SpectWhite, rgbIllum2SpectCyan;
		static SampledSpectrum rgbIllum2SpectMagenta, rgbIllum2SpectYellow;
		static SampledSpectrum rgbIllum2SpectRed, rgbIllum2SpectGreen;
		static SampledSpectrum rgbIllum2SpectBlue;
	};


	class RGBSpectrum : public CoefficientSpectrum<3>
	{
	public:
		RGBSpectrum(float v = 0.f);

		RGBSpectrum(const CoefficientSpectrum<3>& v);

		RGBSpectrum(const RGBSpectrum& s,
		            SpectrumType type = SpectrumType::Reflectance);

		static RGBSpectrum FromRGB(const float rgb[3],
		                           SpectrumType type = SpectrumType::Reflectance);

		void ToRGB(float* rgb) const;

		const RGBSpectrum& ToRGBSpectrum() const;

		static RGBSpectrum FromXYZ(const float xyz[3],
		                           SpectrumType type = SpectrumType::Reflectance);

		void ToXYZ(float* xyz) const;

		static RGBSpectrum FromSampled(const float* lambda, const float* v,
		                               int n);

		float y() const;
	};

	inline Spectrum Lerp(float t, const Spectrum& s1, const Spectrum& s2) {
		return (1 - t) * s1 + t * s2;
	}

	void Blackbody(const float* lambda, int n, float T, float* Le);

	void BlackbodyNormalized(const float* lambda, int n, float T, float* Le);
}

#endif