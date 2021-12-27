#include "reflection.h"
#include "sampling.h"
#include "microfacet.h"

namespace pbrt
{
	float FrDielectric(float cosThetaI, float etaI, float etaT)
	{
		bool entering = cosThetaI > 0.f;
		if (!entering)
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

	Spectrum BxDF::Sample_f(const Vector3f& wo, Vector3f* wi, const Point2f& u, float* pdf, BxDFType* sampledType) const
	{
		*wi = CosineSampleHemisphere(u);
		if (wo.z < 0) wi->z *= -1;
		*pdf = Pdf(wo, *wi);
		return f(wo, *wi);
	}

	float BxDF::Pdf(const Vector3f& wo, const Vector3f& wi) const
	{
		return SameHemisphere(wo, wi) ? AbsCosTheta(wi) * InvPi : 0;
	}

	Spectrum MicrofacetReflection::f(const Vector3f& wo, const Vector3f& wi) const
	{
		float cosThetaO = AbsCosTheta(wo), cosThetaI = AbsCosTheta(wi);;
		Vector3f wh = wi + wo;
		if (cosThetaI == 0 || cosThetaO == 0) return Spectrum(0.);
		if (wh.x == 0 && wh.y == 0 && wh.z == 0) return Spectrum(0.);
		Spectrum F = fresnel->Evaluate(Dot(wi, wh));
		return R * distribution->D(wh) * distribution->G(wo, wi) * F / (4 * cosThetaI * cosThetaO);
	}

	Spectrum MicrofacetReflection::Sample_f(const Vector3f& wo, Vector3f* wi, const Point2f& u, float* pdf, BxDFType* sampledType) const
	{
		Vector3f wh = distribution->Sample_wh(wo, u);
		*wi = Reflect(wo, wh);
		if (!SameHemisphere(wo, *wi)) return { 0.f };
		*pdf = distribution->Pdf(wo, wh) / (4 * Dot(wo, wh));
		return f(wo, *wi);
	}

	float MicrofacetReflection::Pdf(const Vector3f& wo, const Vector3f& wi) const
	{
		if (!SameHemisphere(wo, wi)) return 0;
		Vector3f wh = Normalize(wo + wi);
		return distribution->Pdf(wo, wh) / (4 * Dot(wo, wh));
	}

	Spectrum MicrofacetTransmission::f(const Vector3f& wo, const Vector3f& wi) const
	{
		//if (SameHemisphere(wo, wi)) return 0;  // transmission only

		float cosThetaO = CosTheta(wo);
		float cosThetaI = CosTheta(wi);
		if (cosThetaI == 0 || cosThetaO == 0) return Spectrum(0);

		// Compute $\wh$ from $\wo$ and $\wi$ for microfacet transmission
		float eta = CosTheta(wo) > 0 ? (etaB / etaA) : (etaA / etaB);
		Vector3f wh = Normalize(wo + wi * eta);
		if (wh.z < 0) wh = -wh;

		// Same side?
		if (Dot(wo, wh) * Dot(wi, wh) > 0) return Spectrum(0);

		Spectrum F = fresnel.Evaluate(Dot(wo, wh));

		float sqrtDenom = Dot(wo, wh) + eta * Dot(wi, wh);
		float factor = (mode == TransportMode::Radiance) ? (1 / eta) : 1;

		return (Spectrum(1.f) - F) * T *
			std::abs(distribution->D(wh) * distribution->G(wo, wi) * eta * eta *
				AbsDot(wi, wh) * AbsDot(wo, wh) * factor * factor /
				(cosThetaI * cosThetaO * sqrtDenom * sqrtDenom));
	}

	Spectrum MicrofacetTransmission::Sample_f(const Vector3f& wo, Vector3f* wi, const Point2f& u, float* pdf, BxDFType* sampledType) const
	{
		Vector3f wh = distribution->Sample_wh(wo, u);
		float eta = CosTheta(wo) > 0 ? (etaA / etaB) : (etaB / etaA);
		if (!Refract(wo, (Normal3f)wh, eta, wi)) return 0;
		*pdf = Pdf(wo, *wi);
		return f(wo, *wi);
	}

	float MicrofacetTransmission::Pdf(const Vector3f& wo, const Vector3f& wi) const
	{
		float eta = CosTheta(wo) > 0 ? (etaB / etaA) : (etaA / etaB);
		Vector3f wh = Normalize(wo + wi * eta);
		float sqrtDenom = Dot(wo, wh) + eta * Dot(wi, wh);
		float dwh_dwi = std::abs((eta * eta * Dot(wi, wh)) / (sqrtDenom * sqrtDenom));
		return distribution->Pdf(wo, wh) * dwh_dwi;
	}

	Spectrum FresnelBlend::f(const Vector3f& wo, const Vector3f& wi) const
	{
		auto pow5 = [](float v) { return (v * v) * (v * v) * v; };
		Spectrum diffuse = (28.f / (23.f * Pi)) * Rd *
			(Spectrum(1.f) - Rs) *
			(1 - pow5(1 - .5f * AbsCosTheta(wi))) *
			(1 - pow5(1 - .5f * AbsCosTheta(wo)));
		Vector3f wh = wi + wo;
		if (wh.x == 0 && wh.y == 0 && wh.z == 0) return Spectrum(0);
		wh = Normalize(wh);
		Spectrum specular = distribution->D(wh) /
			(4 * AbsDot(wi, wh) *
				std::max(AbsCosTheta(wi), AbsCosTheta(wo))) *
			SchlickFresnel(Dot(wi, wh));
		return diffuse + specular;
	}
}
