#ifndef PBRT_CORE_REFLECTION_H
#define PBRT_CORE_REFLECTION_H

#include "pbrt.h"
#include "spectrum.h"

namespace pbrt
{
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

    class ScaledBxDF : public BxDF
    {
    public:
        ScaledBxDF(BxDF* bxdf, const Spectrum& scale)
            : BxDF(bxdf->type), bxdf(bxdf), scale(scale) {}
    private:
        BxDF* bxdf;
        Spectrum scale;
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