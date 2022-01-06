#ifndef PBRT_CORE_REFLECTION_H
#define PBRT_CORE_REFLECTION_H

#include "geometry.h"
#include "spectrum.h"
#include "material.h"

namespace pbrt
{
	float FrDielectric(float cosThetaI, float etaI, float etaT);
	Spectrum FrConductor(float cosThetaI, const Spectrum& etai,
	                     const Spectrum& etat, const Spectrum& k);
	inline float CosTheta(const Vector3f& w);
	inline float Cos2Theta(const Vector3f& w);
	inline float AbsCosTheta(const Vector3f& w);

	inline float Sin2Theta(const Vector3f& w);

	inline float SinTheta(const Vector3f& w);

	inline float TanTheta(const Vector3f& w);

	inline float Tan2Theta(const Vector3f& w);

	inline float CosPhi(const Vector3f& w);

	inline float SinPhi(const Vector3f& w);

	inline float Cos2Phi(const Vector3f& w);

	inline float Sin2Phi(const Vector3f& w);

	inline float CosDPhi(const Vector3f& wa, const Vector3f& wb);

	inline Vector3f Reflect(const Vector3f& wo, const Vector3f& n);

	inline bool Refract(const Vector3f& wi, const Normal3f& n, float eta,
	                    Vector3f* wt);

	inline bool SameHemisphere(const Vector3f& w, const Vector3f& wp);

	enum BxDFType
	{
		BSDF_REFLECTION = 1 << 0,
		BSDF_TRANSMISSION = 1 << 1,
		BSDF_DIFFUSE = 1 << 2,
		BSDF_GLOSSY = 1 << 3,
		BSDF_SPECULAR = 1 << 4,
		BSDF_ALL = BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_SPECULAR | BSDF_REFLECTION |
		BSDF_TRANSMISSION,
	};

	struct FourierBSDFTable
	{
		// FourierBSDFTable Public Data
		float eta;
		int mMax;
		int nChannels;
		int nMu;
		float* mu;
		int* m;
		int* aOffset;
		float* a;
		float* a0;
		float* cdf;
		float* recip;

		~FourierBSDFTable();

		static bool Read(const std::string& filename, FourierBSDFTable* table);

		const float* GetAk(int offsetI, int offsetO, int* mptr) const;

		bool GetWeightsAndOffset(float cosTheta, int* offset,
		                         float weights[4]) const;
	};

	class BxDF
	{
	public:
		BxDF(BxDFType type);

		virtual ~BxDF();

		bool MatchesFlags(BxDFType t) const;

		virtual Spectrum f(const Vector3f& wo, const Vector3f& wi) const = 0;
		virtual Spectrum Sample_f(const Vector3f& wo, Vector3f* wi,
		                          const Point2f& u, float* pdf,
		                          BxDFType* sampledType = nullptr) const;
		virtual Spectrum rho(const Vector3f& wo, int nSamples,
		                     const Point2f* samples) const;
		virtual Spectrum rho(int nSamples, const Point2f* samples1, const Point2f* samples2) const;
		virtual float Pdf(const Vector3f& wo, const Vector3f& wi) const;
		const BxDFType type;
	};

	class Fresnel
	{
	public:
		// Fresnel Interface
		virtual ~Fresnel();
		virtual Spectrum Evaluate(float cosThetaI) const = 0;
		//virtual std::string ToString() const = 0;
	};

	class FresnelConductor : public Fresnel
	{
	public:
		FresnelConductor(const Spectrum& etaI, const Spectrum& etaT,
		                 const Spectrum& k);

		Spectrum Evaluate(float cosThetaI) const override;

	private:
		Spectrum etaI, etaT, k;
	};

	class FresnelDielectric : public Fresnel
	{
	public:
		FresnelDielectric(float etaI, float etaT);

		Spectrum Evaluate(float cosThetaI) const override;

	private:
		float etaI, etaT;
	};

	class FresnelNoOp : public Fresnel
	{
	public:
		Spectrum Evaluate(float cosThetaI) const override;
	};

	class ScaledBxDF : public BxDF
	{
	public:
		// ScaledBxDF Public Methods
		ScaledBxDF(BxDF* bxdf, const Spectrum& scale);

		Spectrum rho(const Vector3f& w, int nSamples,
		             const Point2f* samples) const;

		Spectrum rho(int nSamples, const Point2f* samples1,
		             const Point2f* samples2) const;

		Spectrum f(const Vector3f& wo, const Vector3f& wi) const;
		Spectrum Sample_f(const Vector3f& wo, Vector3f* wi, const Point2f& sample,
		                  float* pdf, BxDFType* sampledType) const;
		float Pdf(const Vector3f& wo, const Vector3f& wi) const;
		std::string ToString() const;

	private:
		BxDF* bxdf;
		Spectrum scale;
	};

	class SpecularReflection : public BxDF
	{
	public:
		SpecularReflection(const Spectrum& R, Fresnel* fresnel);

		Spectrum f(const Vector3f& wo, const Vector3f& wi) const override;

		Spectrum Sample_f(const Vector3f& wo, Vector3f* wi, const Point2f& sample, float* pdf,
		                  BxDFType* sampledType) const override;

		float Pdf(const Vector3f& wo, const Vector3f& wi) const override;
	private:
		const Spectrum R;
		const Fresnel* fresnel;
	};

	class SpecularTransmission : public BxDF
	{
	public:
		SpecularTransmission(const Spectrum& T, float etaA, float etaB,
		                     TransportMode mode);

		Spectrum f(const Vector3f& wo, const Vector3f& wi) const override;

		Spectrum Sample_f(const Vector3f& wo, Vector3f* wi, const Point2f& sample, float* pdf,
		                  BxDFType* sampledType) const override;

		float Pdf(const Vector3f& wo, const Vector3f& wi) const override;
	private:
		const Spectrum T;
		const float etaA, etaB;
		const FresnelDielectric fresnel;
		const TransportMode mode;
	};

	class FresnelSpecular : public BxDF
	{
	public:
		FresnelSpecular(const Spectrum& R, const Spectrum& T,
		                float etaA, float etaB, TransportMode mode);

		Spectrum f(const Vector3f& wo, const Vector3f& wi) const override;

		Spectrum Sample_f(const Vector3f& wo, Vector3f* wi, const Point2f& sample, float* pdf,
		                  BxDFType* sampledType) const override;
	private:
		const Spectrum R, T;
		const float etaA, etaB;
		const FresnelDielectric fresnel;
		const TransportMode mode;
	};

	class LambertianReflection : public BxDF
	{
	public:
		LambertianReflection(const Spectrum& R);

		Spectrum f(const Vector3f& wo, const Vector3f& wi) const override;

		Spectrum rho(const Vector3f& wo, int nSamples, const Point2f* samples) const override;

		Spectrum rho(int nSamples, const Point2f* samples1, const Point2f* samples2) const override;

	private:
		const Spectrum R;
	};

	class OrenNayar : public BxDF
	{
	public:
		OrenNayar(const Spectrum& R, float sigma);

		Spectrum f(const Vector3f& wo, const Vector3f& wi) const override;

	private:
		const Spectrum R;
		float A, B;
	};

	class MicrofacetReflection : public BxDF
	{
	public:
		MicrofacetReflection(const Spectrum& r, MicrofacetDistribution* distribution,
		                     Fresnel* fresnel);

		Spectrum f(const Vector3f& wo, const Vector3f& wi) const override;
		Spectrum Sample_f(const Vector3f& wo, Vector3f* wi, const Point2f& u, float* pdf,
		                  BxDFType* sampledType) const override;
		float Pdf(const Vector3f& wo, const Vector3f& wi) const override;
	private:
		const Spectrum R;
		const MicrofacetDistribution* distribution;
		const Fresnel* fresnel;
	};

	class MicrofacetTransmission : public BxDF
	{
	public:
		MicrofacetTransmission(BxDFType type, const Spectrum& t, MicrofacetDistribution* distribution, float etaA,
		                       float etaB, TransportMode mode);

		Spectrum f(const Vector3f& wo, const Vector3f& wi) const override;
		Spectrum Sample_f(const Vector3f& wo, Vector3f* wi, const Point2f& u, float* pdf,
		                  BxDFType* sampledType) const override;
		float Pdf(const Vector3f& wo, const Vector3f& wi) const override;
	private:
		const Spectrum T;
		const MicrofacetDistribution* distribution;
		const float etaA, etaB;
		const FresnelDielectric fresnel;
		const TransportMode mode;
	};

	class FresnelBlend : public BxDF
	{
	public:
		FresnelBlend(const Spectrum& Rd, const Spectrum& Rs,
		             MicrofacetDistribution* distribution);

		Spectrum SchlickFresnel(float cosTheta) const;

		Spectrum Sample_f(const Vector3f& wo, Vector3f* wi, const Point2f& u, float* pdf,
		                  BxDFType* sampledType) const override;
		float Pdf(const Vector3f& wo, const Vector3f& wi) const override;
		Spectrum f(const Vector3f& wo, const Vector3f& wi) const override;
	private:
		const Spectrum Rd, Rs;
		MicrofacetDistribution* distribution;
	};

	class BSDF
	{
	public:
		BSDF(const SurfaceInteraction& si, float eta = 1);

		void Add(BxDF* b);

		int NumComponents(BxDFType flags = BSDF_ALL) const;

		Vector3f WorldToLocal(const Vector3f& v) const;

		Vector3f LocalToWorld(const Vector3f& v) const;

		Spectrum f(const Vector3f& woW, const Vector3f& wiW,
		           BxDFType flags = BSDF_ALL) const;

		Spectrum Sample_f(const Vector3f& woWorld, Vector3f* wiWorld, const Point2f& u,
		                  float* pdf, BxDFType type = BSDF_ALL,
		                  BxDFType* sampledType = nullptr) const;

		Spectrum rho(int nSamples, const Point2f* samples1,
		             const Point2f* samples2, BxDFType flags = BSDF_ALL) const;

		Spectrum rho(const Vector3f& woWorld, int nSamples, const Point2f* samples,
		             BxDFType flags = BSDF_ALL) const;

		float Pdf(const Vector3f& woWorld, const Vector3f& wiWorld,
		          BxDFType flags = BSDF_ALL) const;
		const float eta;
	private:
		const Normal3f ns, ng;
		const Vector3f ss, ts;
		int nBxDFs = 0;
		static constexpr int MaxBxDFs = 8;
		BxDF* bxdfs[MaxBxDFs];
		friend class MixMaterial;

		~BSDF();
	};

	class BSSRDF
	{
	};
}

#endif
