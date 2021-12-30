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
		CoefficientSpectrum(float v = 0.f)
		{
			for (int i = 0; i < nSpectrumSamples; ++i)
				c[i] = v;
		}

		CoefficientSpectrum& operator+=(const CoefficientSpectrum& s2) {
			//DCHECK(!s2.HasNaNs());
			for (int i = 0; i < nSpectrumSamples; ++i) c[i] += s2.c[i];
			return *this;
		}
		CoefficientSpectrum operator+(const CoefficientSpectrum& s2) const {
			//DCHECK(!s2.HasNaNs());
			CoefficientSpectrum ret = *this;
			for (int i = 0; i < nSpectrumSamples; ++i) ret.c[i] += s2.c[i];
			return ret;
		}
		CoefficientSpectrum operator-(const CoefficientSpectrum& s2) const {
			//DCHECK(!s2.HasNaNs());
			CoefficientSpectrum ret = *this;
			for (int i = 0; i < nSpectrumSamples; ++i) ret.c[i] -= s2.c[i];
			return ret;
		}
		CoefficientSpectrum operator/(const CoefficientSpectrum& s2) const {
			//DCHECK(!s2.HasNaNs());
			CoefficientSpectrum ret = *this;
			for (int i = 0; i < nSpectrumSamples; ++i) {
				//CHECK_NE(s2.c[i], 0);
				ret.c[i] /= s2.c[i];
			}
			return ret;
		}
		CoefficientSpectrum operator*(const CoefficientSpectrum& sp) const {
			//DCHECK(!sp.HasNaNs());
			CoefficientSpectrum ret = *this;
			for (int i = 0; i < nSpectrumSamples; ++i) ret.c[i] *= sp.c[i];
			return ret;
		}
		CoefficientSpectrum& operator*=(const CoefficientSpectrum& sp) {
			DCHECK(!sp.HasNaNs());
			for (int i = 0; i < nSpectrumSamples; ++i) c[i] *= sp.c[i];
			return *this;
		}


		CoefficientSpectrum Clamp(float low = 0, float high = Infinity) const
		{
			CoefficientSpectrum ret;
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

		CoefficientSpectrum operator*(float a) const
		{
			CoefficientSpectrum ret = *this;
			for (int i = 0; i < nSpectrumSamples; ++i) ret.c[i] *= a;
			//DCHECK(!ret.HasNaNs());
			return ret;
		}

		CoefficientSpectrum& operator*=(float a)
		{
			for (int i = 0; i < nSpectrumSamples; ++i) c[i] *= a;
			//DCHECK(!HasNaNs());
			return *this;
		}

		friend inline CoefficientSpectrum operator*(float a,
		                                            const CoefficientSpectrum& s)
		{
			//DCHECK(!std::isnan(a) && !s.HasNaNs());
			return s * a;
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

		CoefficientSpectrum& operator/=(float a)
		{
			//CHECK_NE(a, 0);
			//DCHECK(!std::isnan(a));
			for (int i = 0; i < nSpectrumSamples; ++i) c[i] /= a;
			return *this;
		}

		bool operator==(const CoefficientSpectrum& sp) const
		{
			for (int i = 0; i < nSpectrumSamples; ++i)
				if (c[i] != sp.c[i]) return false;
			return true;
		}

		bool operator!=(const CoefficientSpectrum& sp) const
		{
			return !(*this == sp);
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

		float operator[](int i) const {
			// DCHECK(i >= 0 && i < nSpectrumSamples);
			return c[i];
		}

		static const int nSamples = nSpectrumSamples;
	protected:
		float c[nSpectrumSamples];
	};

	class SampledSpectrum : public CoefficientSpectrum<nSpectralSamples>
	{
	public:
		SampledSpectrum(float v = 0.f) : CoefficientSpectrum(v) {}

		SampledSpectrum(const CoefficientSpectrum<nSpectralSamples>& v)
			: CoefficientSpectrum<nSpectralSamples>(v) {}

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

		static void Init();
		void ToXYZ(float xyz[3]) const
		{
			xyz[0] = xyz[1] = xyz[2] = 0.f;
			for (int i = 0; i < nSpectralSamples; ++i)
			{
				xyz[0] += X.c[i] * c[i];
				xyz[1] += Y.c[i] * c[i];
				xyz[2] += Z.c[i] * c[i];
			}
			float scale = float(sampledLambdaEnd - sampledLambdaStart) /
				float(CIE_Y_integral * nSpectralSamples);
			xyz[0] *= scale;
			xyz[1] *= scale;
			xyz[2] *= scale;
		};
		float y() const
		{
			float yy = 0.f;
			for (int i = 0; i < nSpectralSamples; ++i)
			{
				yy += Y.c[i] * c[i];
			}
			return yy * float(sampledLambdaEnd - sampledLambdaStart) /
				float(CIE_Y_integral * nSpectralSamples);
		}

		void ToRGB(float rgb[3]) const
		{
			float xyz[3];
			ToXYZ(xyz);
			XYZToRGB(rgb, xyz);
		}

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
		RGBSpectrum(float v = 0.f) : CoefficientSpectrum<3>(v) {}

		RGBSpectrum(const CoefficientSpectrum<3>& v) : CoefficientSpectrum<3>(v) {}

		RGBSpectrum(const RGBSpectrum& s,
			SpectrumType type = SpectrumType::Reflectance) {
			*this = s;
		}

		static RGBSpectrum FromRGB(const float rgb[3],
			SpectrumType type = SpectrumType::Reflectance)
		{
			RGBSpectrum s;
			s.c[0] = rgb[0];
			s.c[1] = rgb[1];
			s.c[2] = rgb[2];
			return s;
		}

		void ToRGB(float* rgb) const
		{
			rgb[0] = c[0];
			rgb[1] = c[1];
			rgb[2] = c[2];
		}

		const RGBSpectrum& ToRGBSpectrum() const { return *this; }

		static RGBSpectrum FromXYZ(const float xyz[3],
			SpectrumType type = SpectrumType::Reflectance)
		{
			float rgb[3];
			XYZToRGB(xyz, rgb);
			return FromRGB(rgb, type);
		}

		void ToXYZ(float* xyz) const
		{
			return RGBToXYZ(c, xyz);
		}

		static RGBSpectrum FromSampled(const float* lambda, const float* v,
			int n);

		float y() const {
			const float YWeight[3] = { 0.212671f, 0.715160f, 0.072169f };
			return YWeight[0] * c[0] + YWeight[1] * c[1] + YWeight[2] * c[2];
		}
	};

	inline Spectrum Lerp(float t, const Spectrum& s1, const Spectrum& s2) {
		return (1 - t) * s1 + t * s2;
	}

	void Blackbody(const float* lambda, int n, float T, float* Le)
	{
		const float c = 299792458;
		const float h = 6.62606957e-34;
		const float kb = 1.3806488e-23;
		for(int i = 0; i < n; ++i)
		{
			float l = lambda[i] * 1e-9;
			float lambda5 = (l * l) * (l * l) * l;
			Le[i] = (2 * h * c * c) /
				(lambda5 * (std::exp((h * c) / (l * kb * T)) - 1));
		}
	}

	void BlackbodyNormalized(const float* lambda, int n, float T, float* Le)
	{
		Blackbody(lambda, n, T, Le);
		float lambdaMax = 2.8977721e-3 / T * 1e9;
		float maxL;
		Blackbody(&lambdaMax, 1, T, &maxL);
		for (int i = 0; i < n; ++i)
			Le[i] /= maxL;
	}
}

#endif