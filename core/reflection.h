#ifndef PBRT_CORE_REFLECTION_H
#define PBRT_CORE_REFLECTION_H

#include "geometry.h"
#include "interaction.h"
#include "pbrt.h"
#include "spectrum.h"
#include "material.h"

namespace pbrt
{
    float FrDielectric(float cosThetaI, float etaI, float etaT);
    Spectrum FrConductor(float cosThetaI, const Spectrum& etai,
        const Spectrum& etat, const Spectrum& k);
    inline float CosTheta(const Vector3f& w) { return w.z; }
    inline float Cos2Theta(const Vector3f& w) { return w.z * w.z; }
    inline float AbsCosTheta(const Vector3f& w) { return std::abs(w.z); }
    inline float Sin2Theta(const Vector3f& w) {
        return std::max((float)0, (float)1 - Cos2Theta(w));
    }

    inline float SinTheta(const Vector3f& w) { return std::sqrt(Sin2Theta(w)); }

    inline float TanTheta(const Vector3f& w) { return SinTheta(w) / CosTheta(w); }

    inline float Tan2Theta(const Vector3f& w) {
        return Sin2Theta(w) / Cos2Theta(w);
    }

    inline float CosPhi(const Vector3f& w) {
        float sinTheta = SinTheta(w);
        return (sinTheta == 0) ? 1 : Clamp(w.x / sinTheta, -1, 1);
    }

    inline float SinPhi(const Vector3f& w) {
        float sinTheta = SinTheta(w);
        return (sinTheta == 0) ? 0 : Clamp(w.y / sinTheta, -1, 1);
    }

    inline float Cos2Phi(const Vector3f& w) { return CosPhi(w) * CosPhi(w); }

    inline float Sin2Phi(const Vector3f& w) { return SinPhi(w) * SinPhi(w); }

    inline float CosDPhi(const Vector3f& wa, const Vector3f& wb) {
        float waxy = wa.x * wa.x + wa.y * wa.y;
        float wbxy = wb.x * wb.x + wb.y * wb.y;
        if (waxy == 0 || wbxy == 0)
            return 1;
        return Clamp((wa.x * wb.x + wa.y * wb.y) / std::sqrt(waxy * wbxy), -1, 1);
    }
    inline Vector3f Reflect(const Vector3f& wo, const Vector3f& n) {
        return -wo + 2 * Dot(wo, n) * n;
    }

    inline bool Refract(const Vector3f& wi, const Normal3f& n, float eta,
                        Vector3f* wt)
    {
        float cosThetaI = Dot(n, wi);
        float sin2ThetaI = std::max(0.f, 1.f - cosThetaI * cosThetaI);
        float sin2ThetaT = eta * eta * sin2ThetaI;
        if (sin2ThetaI >= 1) return false;
        float cosThetaT = std::sqrt(1 - sin2ThetaT);
        *wt = eta * (-wi) + (eta * cosThetaI - cosThetaT) * Vector3f(n);
        return true;
    }

    inline bool SameHemisphere(const Vector3f& w, const Vector3f& wp)
    {
        return w.z * wp.z > 0;
    }

    enum BxDFType {
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

	    ~FourierBSDFTable()
	    {
		    delete[] mu;
		    delete[] m;
		    delete[] aOffset;
		    delete[] a;
		    delete[] a0;
		    delete[] cdf;
		    delete[] recip;
	    }

	    static bool Read(const std::string& filename, FourierBSDFTable* table);

	    const float* GetAk(int offsetI, int offsetO, int* mptr) const
	    {
		    *mptr = m[offsetO * nMu + offsetI];
		    return a + aOffset[offsetO * nMu + offsetI];
	    }

	    bool GetWeightsAndOffset(float cosTheta, int* offset,
	                             float weights[4]) const;
    };

    class BxDF
    {
    public:
        BxDF(BxDFType type) : type(type) {}
        virtual ~BxDF();
        bool MatchesFlags(BxDFType t) const
        {
            return (t & type) == type;
        }
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

    class Fresnel {
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
            const Spectrum& k) : etaI(etaI), etaT(etaT), k(k) { }
        Spectrum Evaluate(float cosThetaI) const override
        {
            return FrConductor(std::abs(cosThetaI), etaI, etaT, k);
        }
    private:
        Spectrum etaI, etaT, k;
    };

    class FresnelDielectric : public Fresnel
    {
    public:
        FresnelDielectric(float etaI, float etaT): etaI(etaI), etaT(etaT) { }
        Spectrum Evaluate(float cosThetaI) const override
        {
            return FrDielectric(cosThetaI, etaI, etaT);
        }
    private:
        float etaI, etaT;
    };

    class FresnelNoOp : public Fresnel
    {
    public:
        Spectrum Evaluate(float cosThetaI) const override { return {1.}; }
    };

    class ScaledBxDF : public BxDF
    {
    public:
        // ScaledBxDF Public Methods
        ScaledBxDF(BxDF* bxdf, const Spectrum& scale)
            : BxDF(BxDFType(bxdf->type)), bxdf(bxdf), scale(scale) {}
        Spectrum rho(const Vector3f& w, int nSamples,
            const Point2f* samples) const {
            return scale * bxdf->rho(w, nSamples, samples);
        }
        Spectrum rho(int nSamples, const Point2f* samples1,
            const Point2f* samples2) const {
            return scale * bxdf->rho(nSamples, samples1, samples2);
        }
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
	    SpecularReflection(const Spectrum& R, Fresnel* fresnel)
		    : BxDF(BxDFType(BSDF_REFLECTION | BSDF_SPECULAR)),
		      R(R), fresnel(fresnel)
	    { }
        Spectrum f(const Vector3f& wo, const Vector3f& wi) const override
	    {
            return {0.f};
	    }
        Spectrum Sample_f(const Vector3f& wo, Vector3f* wi, const Point2f& sample, float* pdf, BxDFType* sampledType) const override
	    {
            *wi = Vector3f(-wo.x, -wo.y, wo.z);
            *pdf = 1;
            return fresnel->Evaluate(CosTheta(*wi)) * R / AbsCosTheta(*wi);
	    }
        float Pdf(const Vector3f& wo, const Vector3f& wi) const override { return 0.f; }
    private:
	    const Spectrum R;
        const Fresnel* fresnel;
    };

    class SpecularTransmission : public BxDF
    {
    public:
        SpecularTransmission(const Spectrum& T, float etaA, float etaB,
                             TransportMode mode):
	        BxDF(BxDFType(BSDF_TRANSMISSION | BSDF_SPECULAR)), T(T),
	        etaA(etaA), etaB(etaB), fresnel(etaA, etaB), mode(mode)
        {
        }

        Spectrum f(const Vector3f& wo, const Vector3f& wi) const override
        {
	        return {0.f};
        }

        Spectrum Sample_f(const Vector3f& wo, Vector3f* wi, const Point2f& sample, float* pdf,
                          BxDFType* sampledType) const override
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
        float Pdf(const Vector3f& wo, const Vector3f& wi) const override { return 0.f; }
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
	                    float etaA, float etaB, TransportMode mode)
		    : BxDF(BxDFType(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_SPECULAR)),
		      R(R), T(T), etaA(etaA), etaB(etaB), fresnel(etaA, etaB),
		      mode(mode)
	    {
	    }

        Spectrum f(const Vector3f& wo, const Vector3f& wi) const override { return { 0.f }; }

        Spectrum Sample_f(const Vector3f& wo, Vector3f* wi, const Point2f& sample, float* pdf, BxDFType* sampledType) const override;
    private:
	    const Spectrum R, T;
	    const float etaA, etaB;
	    const FresnelDielectric fresnel;
	    const TransportMode mode;
    };

    class LambertianReflection : public BxDF
    {
    public:
        LambertianReflection(const Spectrum& R)
	        :BxDF(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)), R(R){ }
        Spectrum f(const Vector3f& wo, const Vector3f& wi) const override
        {
            return R * InvPi;
        }
        Spectrum rho(const Vector3f& wo, int nSamples, const Point2f* samples) const override
        {
            return R;
        }
        Spectrum rho(int nSamples, const Point2f* samples1, const Point2f* samples2) const override
        {
            return R;
        }
    private:
        const Spectrum R;
    };

    class OrenNayar : public BxDF
    {
    public:
        OrenNayar(const Spectrum& R, float sigma)
	        : BxDF(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)), R(R)
        {
            sigma = Radians(sigma);
            float sigma2 = sigma * sigma;
            A = 1.f - sigma2 / (2.f * (sigma2 + .33f));
            B = .45f * sigma2 / (sigma2 + .09f);
        }
        Spectrum f(const Vector3f& wo, const Vector3f& wi) const override
        {
            float sinThetaI = SinTheta(wi);
            float sinThetaO = SinTheta(wo);
            float maxCos = 0;
            if (sinThetaI > 1e-4 && sinThetaO > 1e-4) {
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
    private:
        const Spectrum R;
        float A, B;
    };

    class MicrofacetReflection : public BxDF
    {
    public:
	    MicrofacetReflection(const Spectrum& r, MicrofacetDistribution* distribution,
		    Fresnel* fresnel)
		    : BxDF(BxDFType(BSDF_REFLECTION | BSDF_GLOSSY)),
		      R(r),
		      distribution(distribution),
		      fresnel(fresnel) { }
        Spectrum f(const Vector3f& wo, const Vector3f& wi) const override;
        Spectrum Sample_f(const Vector3f& wo, Vector3f* wi, const Point2f& u, float* pdf, BxDFType* sampledType) const override;
	    virtual Vector3f Sample_wh(const Vector3f& wo, const Point2f& u) const = 0;
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
		    float etaB, TransportMode mode)
		    : BxDF(BxDFType(BSDF_TRANSMISSION | BSDF_GLOSSY)),
		      T(t),
		      distribution(distribution),
		      etaA(etaA),
		      etaB(etaB),
		      fresnel(etaA, etaB),
		      mode(mode)
	    {
	    }
        Spectrum f(const Vector3f& wo, const Vector3f& wi) const override;
        Spectrum Sample_f(const Vector3f& wo, Vector3f* wi, const Point2f& u, float* pdf, BxDFType* sampledType) const override;
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
	                 MicrofacetDistribution* distribution)
		    : BxDF(BxDFType(BSDF_REFLECTION | BSDF_GLOSSY)),
		      Rd(Rd), Rs(Rs), distribution(distribution)
	    {
	    }

	    Spectrum SchlickFresnel(float cosTheta) const
	    {
		    auto pow5 = [](float v) { return (v * v) * (v * v) * v; };
		    return Rs + pow5(1 - cosTheta) * (Spectrum(1.) - Rs);
	    }
        Spectrum Sample_f(const Vector3f& wo, Vector3f* wi, const Point2f& u, float* pdf, BxDFType* sampledType) const override;
        float Pdf(const Vector3f& wo, const Vector3f& wi) const override;
	    Spectrum f(const Vector3f& wo, const Vector3f& wi) const override;
    private:
        const Spectrum Rd, Rs;
        MicrofacetDistribution* distribution;
    };

	class BSDF {
	public:
        BSDF(const SurfaceInteraction& si, float eta = 1)
	        : eta(eta), ns(si.shading.n), ng(si.n),
        ss(Normalize(si.shading.dpdu)), ts(Cross(ns, ss)) { }
        void Add(BxDF* b)
        {
            assert(nBxDFs < MaxBxDFs);
            bxdfs[nBxDFs++] = b;
        }
        int NumComponents(BxDFType flags = BSDF_ALL) const
        {
            int num = 0;
            for (int i = 0; i < nBxDFs; ++i)
                if (bxdfs[i]->MatchesFlags(flags)) ++num;
            return num;
        }

        Vector3f WorldToLocal(const Vector3f& v) const
        {
	        return {Dot(v, ss), Dot(v, ts), Dot(v, ns)};
        }

        Vector3f LocalToWorld(const Vector3f& v) const
        {
	        return
	        {
		        ss.x * v.x + ts.x * v.y + ns.x * v.z,
		        ss.y * v.x + ts.y * v.y + ns.y * v.z,
		        ss.z * v.x + ts.z * v.y + ns.z * v.z,
	        };
        }

		Spectrum f(const Vector3f& woW, const Vector3f& wiW,
			BxDFType flags = BSDF_ALL) const
        {
            Vector3f wi = WorldToLocal(wiW), wo = WorldToLocal(woW);
            bool reflect = Dot(wiW, ng) * Dot(woW, ng) > 0;
            Spectrum f(0.f);
            for (int i = 0; i < nBxDFs; ++i)
	            if (bxdfs[i]->MatchesFlags(flags) &&
		            (reflect && (bxdfs[i]->type && BSDF_REFLECTION)) ||
		            (!reflect && (bxdfs[i]->type && BSDF_TRANSMISSION)))
		            f += bxdfs[i]->f(wo, wi);
        }
        Spectrum Sample_f(const Vector3f& woWorld, Vector3f* wiWorld, const Point2f& u,
            float* pdf, BxDFType type = BSDF_ALL,
            BxDFType* sampledType = nullptr) const;
        Spectrum rho(int nSamples, const Point2f* samples1,
            const Point2f* samples2, BxDFType flags = BSDF_ALL) const
        {
            Spectrum ret(0.f);
            for (int i = 0; i < nBxDFs; ++i)
                if (bxdfs[i]->MatchesFlags(flags))
                    ret += bxdfs[i]->rho(nSamples, samples1, samples2);
            return ret;
        }
        Spectrum rho(const Vector3f& woWorld, int nSamples, const Point2f* samples,
            BxDFType flags = BSDF_ALL) const
        {
            Vector3f wo = WorldToLocal(woWorld);
            Spectrum ret(0.f);
            for (int i = 0; i < nBxDFs; ++i)
                if (bxdfs[i]->MatchesFlags(flags))
                    ret += bxdfs[i]->rho(wo, nSamples, samples);
            return ret;
        }
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
        ~BSDF() {}
	};
    class BSSRDF {};
}

#endif