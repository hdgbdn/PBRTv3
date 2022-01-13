#include "reflection.h"
#include "sampling.h"
#include "microfacet.h"
#include "interaction.h"

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


	FourierBSDFTable::~FourierBSDFTable()
	{
		delete[] mu;
		delete[] m;
		delete[] aOffset;
		delete[] a;
		delete[] a0;
		delete[] cdf;
		delete[] recip;
	}

	const float* FourierBSDFTable::GetAk(int offsetI, int offsetO, int* mptr) const
	{
		*mptr = m[offsetO * nMu + offsetI];
		return a + aOffset[offsetO * nMu + offsetI];
	}

	BxDF::BxDF(BxDFType type): type(type)
	{
	}

	bool BxDF::MatchesFlags(BxDFType t) const
	{
		return (t & type) == type;
	}

	FresnelConductor::FresnelConductor(const Spectrum& etaI, const Spectrum& etaT, const Spectrum& k): etaI(etaI), etaT(etaT), k(k)
	{
	}

	Spectrum FresnelConductor::Evaluate(float cosThetaI) const
	{
		return FrConductor(std::abs(cosThetaI), etaI, etaT, k);
	}

	FresnelDielectric::FresnelDielectric(float etaI, float etaT): etaI(etaI), etaT(etaT)
	{
	}

	Spectrum FresnelDielectric::Evaluate(float cosThetaI) const
	{
		return FrDielectric(cosThetaI, etaI, etaT);
	}

	Spectrum FresnelNoOp::Evaluate(float cosThetaI) const
	{ return {1.}; }

	ScaledBxDF::ScaledBxDF(BxDF* bxdf, const Spectrum& scale): BxDF(BxDFType(bxdf->type)), bxdf(bxdf), scale(scale)
	{
	}

	Spectrum ScaledBxDF::rho(const Vector3f& w, int nSamples, const Point2f* samples) const
	{
		return scale * bxdf->rho(w, nSamples, samples);
	}

	Spectrum ScaledBxDF::rho(int nSamples, const Point2f* samples1, const Point2f* samples2) const
	{
		return scale * bxdf->rho(nSamples, samples1, samples2);
	}

    Spectrum ScaledBxDF::f(const Vector3f &wo, const Vector3f &wi) const
    {
        // TODO implement
        return pbrt::Spectrum();
    }

    Spectrum ScaledBxDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample, float *pdf,
                                  BxDFType *sampledType) const
    {
        // TODO implement
        return BxDF::Sample_f(wo, wi, sample, pdf, sampledType);
    }

    float ScaledBxDF::Pdf(const Vector3f& wo, const Vector3f& wi) const
    {
        // TODO implement
        return 1.f;
    }

    SpecularReflection::SpecularReflection(const Spectrum& R, Fresnel* fresnel): BxDF(BxDFType(BSDF_REFLECTION | BSDF_SPECULAR)),
		R(R), fresnel(fresnel)
	{
	}

	Spectrum SpecularReflection::f(const Vector3f& wo, const Vector3f& wi) const
	{
		return {0.f};
	}

	Spectrum SpecularReflection::Sample_f(const Vector3f& wo, Vector3f* wi, const Point2f& sample, float* pdf,
		BxDFType* sampledType) const
	{
		*wi = Vector3f(-wo.x, -wo.y, wo.z);
		*pdf = 1;
		return fresnel->Evaluate(CosTheta(*wi)) * R / AbsCosTheta(*wi);
	}

	float SpecularReflection::Pdf(const Vector3f& wo, const Vector3f& wi) const
	{ return 0.f; }

	SpecularTransmission::SpecularTransmission(const Spectrum& T, float etaA, float etaB, TransportMode mode):
		BxDF(BxDFType(BSDF_TRANSMISSION | BSDF_SPECULAR)), T(T),
		etaA(etaA), etaB(etaB), fresnel(etaA, etaB), mode(mode)
	{
	}

	Spectrum SpecularTransmission::f(const Vector3f& wo, const Vector3f& wi) const
	{
		return {0.f};
	}

	Spectrum SpecularTransmission::Sample_f(const Vector3f& wo, Vector3f* wi, const Point2f& sample, float* pdf,
		BxDFType* sampledType) const
	{
		bool entering = CosTheta(wo) > 0;
		float etaI = entering ? etaA : etaB;
		float etaT = entering ? etaB : etaA;
		if (!Refract(wo, Faceforward(Normal3f(0, 0, 1), wo), etaI / etaT, wi))
			return 0;
		*pdf = 1;
		Spectrum ft = T * (Spectrum(1.) - fresnel.Evaluate(CosTheta(*wi)));
		if (mode == TransportMode::Radiance)
			ft *= (etaI * etaI) / (etaT * etaT);

		return ft / AbsCosTheta(*wi);
	}

	float SpecularTransmission::Pdf(const Vector3f& wo, const Vector3f& wi) const
	{ return 0.f; }

	FresnelSpecular::FresnelSpecular(const Spectrum& R, const Spectrum& T, float etaA, float etaB, TransportMode mode): BxDF(BxDFType(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_SPECULAR)),
		R(R), T(T), etaA(etaA), etaB(etaB), fresnel(etaA, etaB),
		mode(mode)
	{
	}

	Spectrum FresnelSpecular::f(const Vector3f& wo, const Vector3f& wi) const
	{ return {0.f}; }

	LambertianReflection::LambertianReflection(const Spectrum& R): BxDF(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)), R(R)
	{
	}

	Spectrum LambertianReflection::f(const Vector3f& wo, const Vector3f& wi) const
	{
		return R * InvPi;
	}

	Spectrum LambertianReflection::rho(const Vector3f& wo, int nSamples, const Point2f* samples) const
	{
		return R;
	}

	Spectrum LambertianReflection::rho(int nSamples, const Point2f* samples1, const Point2f* samples2) const
	{
		return R;
	}

	OrenNayar::OrenNayar(const Spectrum& R, float sigma): BxDF(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)), R(R)
	{
		sigma = Radians(sigma);
		float sigma2 = sigma * sigma;
		A = 1.f - sigma2 / (2.f * (sigma2 + .33f));
		B = .45f * sigma2 / (sigma2 + .09f);
	}

	Spectrum OrenNayar::f(const Vector3f& wo, const Vector3f& wi) const
	{
		float sinThetaI = SinTheta(wi);
		float sinThetaO = SinTheta(wo);
		float maxCos = 0;
		if (sinThetaI > 1e-4 && sinThetaO > 1e-4)
		{
			float sinPhiI = SinPhi(wi), cosPhiI = CosPhi(wi);
			float sinPhiO = SinPhi(wo), cosPhiO = CosPhi(wo);
			float dCos = cosPhiI * cosPhiO + sinPhiI * sinPhiO;
			maxCos = std::max((float)0, dCos);
		}
		float sinAlpha, tanBeta;
		if (AbsCosTheta(wi) > AbsCosTheta(wo))
		{
			sinAlpha = sinThetaO;
			tanBeta = sinThetaI / AbsCosTheta(wi);
		}
		else
		{
			sinAlpha = sinThetaI;
			tanBeta = sinThetaO / AbsCosTheta(wo);
		}
		return R * InvPi * (A + B * maxCos * sinAlpha * tanBeta);
	}

	MicrofacetReflection::MicrofacetReflection(const Spectrum& r, MicrofacetDistribution* distribution,
		Fresnel* fresnel): BxDF(BxDFType(BSDF_REFLECTION | BSDF_GLOSSY)),
		                   R(r),
		                   distribution(distribution),
		                   fresnel(fresnel)
	{
	}

	MicrofacetTransmission::MicrofacetTransmission(BxDFType type, const Spectrum& t,
		MicrofacetDistribution* distribution, float etaA, float etaB, TransportMode mode): BxDF(BxDFType(BSDF_TRANSMISSION | BSDF_GLOSSY)),
		T(t),
		distribution(distribution),
		etaA(etaA),
		etaB(etaB),
		fresnel(etaA, etaB),
		mode(mode)
	{
	}

	FresnelBlend::FresnelBlend(const Spectrum& Rd, const Spectrum& Rs, MicrofacetDistribution* distribution): BxDF(BxDFType(BSDF_REFLECTION | BSDF_GLOSSY)),
		Rd(Rd), Rs(Rs), distribution(distribution)
	{
	}

	Spectrum FresnelBlend::SchlickFresnel(float cosTheta) const
	{
		auto pow5 = [](float v) { return (v * v) * (v * v) * v; };
		return Rs + pow5(1 - cosTheta) * (Spectrum(1.) - Rs);
	}

	BSDF::BSDF(const SurfaceInteraction& si, float eta): eta(eta), ns(si.shading.n), ng(si.n),
	                                                     ss(Normalize(si.shading.dpdu)), ts(Cross(ns, ss))
	{
	}

	void BSDF::Add(BxDF* b)
	{
		assert(nBxDFs < MaxBxDFs);
		bxdfs[nBxDFs++] = b;
	}

	int BSDF::NumComponents(BxDFType flags) const
	{
		int num = 0;
		for (int i = 0; i < nBxDFs; ++i)
			if (bxdfs[i]->MatchesFlags(flags)) ++num;
		return num;
	}

	Vector3f BSDF::WorldToLocal(const Vector3f& v) const
	{
		return {Dot(v, ss), Dot(v, ts), Dot(v, ns)};
	}

	Vector3f BSDF::LocalToWorld(const Vector3f& v) const
	{
		return
		{
			ss.x * v.x + ts.x * v.y + ns.x * v.z,
			ss.y * v.x + ts.y * v.y + ns.y * v.z,
			ss.z * v.x + ts.z * v.y + ns.z * v.z,
		};
	}

	Spectrum BSDF::f(const Vector3f& woW, const Vector3f& wiW, BxDFType flags) const
	{
		Vector3f wi = WorldToLocal(wiW), wo = WorldToLocal(woW);
		bool reflect = Dot(wiW, ng) * Dot(woW, ng) > 0;
		Spectrum f(0.f);
		for (int i = 0; i < nBxDFs; ++i)
			if (bxdfs[i]->MatchesFlags(flags) &&
				(reflect && (bxdfs[i]->type && BSDF_REFLECTION)) ||
				(!reflect && (bxdfs[i]->type && BSDF_TRANSMISSION)))
				f += bxdfs[i]->f(wo, wi);
		return f;
	}

	Spectrum BSDF::rho(int nSamples, const Point2f* samples1, const Point2f* samples2, BxDFType flags) const
	{
		Spectrum ret(0.f);
		for (int i = 0; i < nBxDFs; ++i)
			if (bxdfs[i]->MatchesFlags(flags))
				ret += bxdfs[i]->rho(nSamples, samples1, samples2);
		return ret;
	}

	Spectrum BSDF::rho(const Vector3f& woWorld, int nSamples, const Point2f* samples, BxDFType flags) const
	{
		Vector3f wo = WorldToLocal(woWorld);
		Spectrum ret(0.f);
		for (int i = 0; i < nBxDFs; ++i)
			if (bxdfs[i]->MatchesFlags(flags))
				ret += bxdfs[i]->rho(wo, nSamples, samples);
		return ret;
	}

	BSDF::~BSDF()
	{
	}

	Spectrum BxDF::Sample_f(const Vector3f& wo, Vector3f* wi, const Point2f& u, float* pdf, BxDFType* sampledType) const
	{
		*wi = CosineSampleHemisphere(u);
		if (wo.z < 0) wi->z *= -1;
		*pdf = Pdf(wo, *wi);
		return f(wo, *wi);
	}

	Spectrum BxDF::rho(const Vector3f& wo, int nSamples, const Point2f* samples) const
	{
		Spectrum r(0.f);
		for(int i = 0; i < nSamples; ++i)
		{
			Vector3f wi;
			float pdf = 0;
			Spectrum f = Sample_f(wo, &wi, samples[i], &pdf);
			if (pdf > 0) r += f * AbsCosTheta(wi) / pdf;
		}
		return r / nSamples;
	}

	Spectrum BxDF::rho(int nSamples, const Point2f* samples1, const Point2f* samples2) const
	{
		Spectrum r(0.f);
		for(int i = 0; i < nSamples; ++i)
		{
			Vector3f wo, wi;
			wo = UniformSampleHemisphere(samples1[i]);
			float pdfo = UniformHemispherePdf(), pdfi = 0;
			Spectrum f = Sample_f(wo, &wi, samples2[i], &pdfi);
			if (pdfi > 0)
				r += f * AbsCosTheta(wi) * AbsCosTheta(wo) / (pdfo * pdfi);
		}
		return r / (Pi * nSamples);
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

	Spectrum FresnelBlend::Sample_f(const Vector3f& wo, Vector3f* wi, const Point2f& uOrig, float* pdf, BxDFType* sampledType) const
	{
		Point2f u = uOrig;
		if(u[0] < .5)
		{
			u[0] = 2 * u[0];
			*wi = CosineSampleHemisphere(u);
			if (wo.z < 0) wi->z *= -1;
		}
		else
		{
			u[0] = 2 * (u[0] - .5f);
			Vector3f wh = distribution->Sample_wh(wo, u);
			*wi = Reflect(wo, wh);
			if (!SameHemisphere(wo, *wi)) return Spectrum(0.f);
		}
		*pdf = Pdf(wo, *wi);
		return f(wo, *wi);
	}

	float FresnelBlend::Pdf(const Vector3f& wo, const Vector3f& wi) const
	{
		if (!SameHemisphere(wo, wi)) return 0;
		Vector3f wh = Normalize(wo + wi);
		float pdf_wh = distribution->Pdf(wo, wh);
		return .5f * (AbsCosTheta(wi) * InvPi +
			pdf_wh / (4 * Dot(wo, wh)));
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
	Spectrum FresnelSpecular::Sample_f(const Vector3f& wo, Vector3f* wi, const Point2f& sample, float* pdf, BxDFType* sampledType) const
	{
		float F = FrDielectric(CosTheta(wo), etaA, etaB);
		if(sample[0] < F)
		{
			*wi = Vector3f(-wo.x, -wo.y, wo.z);
			if (sampledType) *sampledType = BxDFType(BSDF_SPECULAR | BSDF_REFLECTION);
			*pdf = F;
			return F * R / AbsCosTheta(*wi);
		}
		else
		{
			bool entering = CosTheta(wo) > 0;
			float etaI = entering ? etaA : etaB;
			float etaT = entering ? etaB : etaA;
			if (!Refract(wo, Faceforward(Normal3f(0, 0, 1), wo), etaI / etaT, wi))
				return 0;
			Spectrum ft = T * (1 - F);
			if (mode == TransportMode::Radiance)
				ft *= (etaI * etaI) / (etaT * etaT);
			if (sampledType)
				*sampledType = BxDFType(BSDF_SPECULAR | BSDF_TRANSMISSION);
			*pdf = 1 - F;
			return ft / AbsCosTheta(*wi);
		}
	}
	Spectrum BSDF::Sample_f(const Vector3f& woWorld, Vector3f* wiWorld, const Point2f& u, float* pdf, BxDFType type, BxDFType* sampledType) const
	{
		int matchingComps = NumComponents(type);
		if(matchingComps == 0)
		{
			*pdf = 0;
			return { 0 };
		}
		int comp = std::min((int)std::floor(u[0] * matchingComps), matchingComps - 1);
		BxDF* bxdf = nullptr;
		int count = comp;
		for(int i = 0; i < nBxDFs; ++i)
			if(bxdfs[i]->MatchesFlags(type) && count-- == 0)
			{
				bxdf = bxdfs[i];
				break;
			}
		Point2f uRemapped(u[0] * matchingComps - comp, u[1]);
		Vector3f wo = WorldToLocal(woWorld), wi;
		*pdf = 0;
		if (sampledType) *sampledType = bxdf->type;
		Spectrum f = bxdf->Sample_f(wo, &wi, uRemapped, pdf, sampledType);
		if (*pdf == 0) return { 0 };
		*wiWorld = LocalToWorld(wi);
		if (!(bxdf->type & BSDF_SPECULAR) && matchingComps > 1)
			for (int i = 0; i < nBxDFs; ++i)
				if (bxdfs[i] != bxdf && bxdfs[i]->MatchesFlags(type))
					*pdf += bxdfs[i]->Pdf(wo, wi);
		if (matchingComps > 1) *pdf /= matchingComps;
		if(!(bxdf->type & BSDF_SPECULAR) && matchingComps > 1)
		{
			bool reflect = Dot(*wiWorld, ng) * Dot(woWorld, ng) > 0;
			f = 0.;
			for (int i = 0; i < nBxDFs; ++i)
				if (bxdfs[i]->MatchesFlags(type) &&
					((reflect && (bxdfs[i]->type & BSDF_REFLECTION)) ||
						(!reflect && (bxdfs[i]->type & BSDF_TRANSMISSION))))
					f += bxdfs[i]->f(wo, wi);
		}
		return f;
	}
	float BSDF::Pdf(const Vector3f& woWorld, const Vector3f& wiWorld, BxDFType flags) const
	{
		if (nBxDFs == 0.f) return 0.f;
		Vector3f wo = WorldToLocal(woWorld), wi = WorldToLocal(wiWorld);
		if (wo.z == 0) return 0.;
		float pdf = 0.f;
		int matchingComps = 0;
		for (int i = 0; i < nBxDFs; ++i)
			if (bxdfs[i]->MatchesFlags(flags)) {
				++matchingComps;
				pdf += bxdfs[i]->Pdf(wo, wi);
			}
		float v = matchingComps > 0 ? pdf / matchingComps : 0.f;
		return v;
	}
}
