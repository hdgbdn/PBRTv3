#ifndef PBRT_CORE_REFLECTION_H
#define PBRT_CORE_REFLECTION_H

#include "geometry.h"
#include "pbrt.h"
#include "spectrum.h"

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

    enum BxDFType {
        BSDF_REFLECTION = 1 << 0,
        BSDF_TRANSMISSION = 1 << 1,
        BSDF_DIFFUSE = 1 << 2,
        BSDF_GLOSSY = 1 << 3,
        BSDF_SPECULAR = 1 << 4,
        BSDF_ALL = BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_SPECULAR | BSDF_REFLECTION |
        BSDF_TRANSMISSION,
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
                                  const Point2f& sample, float* pdf,
                                  BxDFType* sampledType = nullptr) const;
        virtual Spectrum rho(const Vector3f& wo, int nSamples,
                             const Point2f* samples) const;
        virtual Spectrum rho(int nSamples, const Point2f* samples1, const Point2f* samples2) const;

        const BxDFType type;
    };

    class Fresnel {
    public:
        // Fresnel Interface
        virtual ~Fresnel();
        virtual Spectrum Evaluate(float cosThetaI) const = 0;
        virtual std::string ToString() const = 0;
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
        ScaledBxDF(BxDF* bxdf, const Spectrum& scale)
            : BxDF(bxdf->type), bxdf(bxdf), scale(scale) {}
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
    private:
	    const Spectrum R;
        const Fresnel* fresnel;
    };

	class BSDF {
	public:
		Spectrum f(const Vector3f& woW, const Vector3f& wiW,
			BxDFType flags = BSDF_ALL) const;
        Spectrum Sample_f(const Vector3f& wo, Vector3f* wi, const Point2f& u,
            float* pdf, BxDFType type = BSDF_ALL,
            BxDFType* sampledType = nullptr) const;
	};
    class BSSRDF {};
}

#endif