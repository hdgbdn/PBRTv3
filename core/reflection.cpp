#include "reflection.h"

namespace pbrt
{
	float FrDielectric(float cosThetaI, float etaI, float etaT)
	{
		bool entering = cosThetaI > 0.f;
		if(!entering)
		{
			std::swap(etaI, etaT);
			cosThetaI = std::abs(cosThetaI);
		}
		float sinThetaI = std::sqrt(std::max(float(0), 1 - cosThetaI * cosThetaI));
		float sinThetaT = etaI / etaT * sinThetaI;
		if (sinThetaT > 1.f) return 1;
		float cosThetaT = std::sqrt(std::max(float(0), 1 - sinThetaI * sinThetaI));
		float Rparl = ((etaT * cosThetaI) - (etaI * cosThetaT)) /
			((etaT * cosThetaI) + (etaI * cosThetaT));
		float Rperp = ((etaI * cosThetaI) - (etaT * cosThetaT)) /
			((etaI * cosThetaI) + (etaT * cosThetaT));
		return (Rparl * Rparl + Rperp * Rperp) / 2;
	}
	Spectrum FrConductor(float cosThetaI, const Spectrum& etai, const Spectrum& etat, const Spectrum& k)
	{
		cosThetaI = Clamp(cosThetaI, -1, 1);
		Spectrum eta = etat / etai;
		Spectrum etak = k / etai;

		float cosThetaI2 = cosThetaI * cosThetaI;
		float sinThetaI2 = 1. - cosThetaI2;
		Spectrum eta2 = eta * eta;
		Spectrum etak2 = etak * etak;

		Spectrum t0 = eta2 - etak2 - sinThetaI2;
		Spectrum a2plusb2 = Sqrt(t0 * t0 + 4 * eta2 * etak2);
		Spectrum t1 = a2plusb2 + cosThetaI2;
		Spectrum a = Sqrt(0.5f * (a2plusb2 + t0));
		Spectrum t2 = (float)2 * cosThetaI * a;
		Spectrum Rs = (t1 - t2) / (t1 + t2);

		Spectrum t3 = cosThetaI2 * a2plusb2 + sinThetaI2 * sinThetaI2;
		Spectrum t4 = t2 * sinThetaI2;
		Spectrum Rp = Rs * (t3 - t4) / (t3 + t4);

		return 0.5 * (Rp + Rs);
	}
}
