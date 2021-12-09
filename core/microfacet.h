#ifndef PBRT_CORE_FILM_H
#define PBRT_CORE_FILM_H

#include "pbrt.h"
#include "geometry.h"
#include "reflection.h"

namespace pbrt
{
	class MicrofacetDistribution
	{
    public:
        virtual float D(const Vector3f &wh) const = 0;
        virtual float Lambda(const Vector3f &w) const = 0;
        float G1(const Vector3f &w) const {
            return 1 / (1 + Lambda(w));
        }
        float G(const Vector3f &wo, const Vector3f &wi) const {
            return 1 / (1 + Lambda(wo) + Lambda(wi));
        }
        virtual Vector3f Sample_wh(const Vector3f &wo,
                                   const Point2f &u) const = 0;
        float Pdf(const Vector3f &wo, const Vector3f &wh) const;
    protected:
        MicrofacetDistribution(bool sampleVisibleArea)
                : sampleVisibleArea(sampleVisibleArea) { }
        const bool sampleVisibleArea;
    };

    class BeckmannDistribution : public MicrofacetDistribution
	{
    public:
        static float RoughnessToAlpha(float roughness)
        {
            roughness = std::max(roughness, (float)1e-3);
            float x = std::log(roughness);
            return 1.62142f + 0.819955f * x + 0.1734f * x * x +
                0.0171201f * x * x * x + 0.000640711f * x * x * x * x;
        }
        BeckmannDistribution(float alphaX, float alphaY, bool samplevis = true)
	        : MicrofacetDistribution(samplevis), alphaX(alphaX), alphaY(alphaY) { }
        float D(const Vector3f& wh) const
        {
            float tan2Theta = Tan2Theta(wh);
            if (std::isinf(tan2Theta)) return 0.;
            float cos4Theta = Cos2Theta(wh) * Cos2Theta(wh);
            return std::exp(-tan2Theta * (Cos2Phi(wh) / (alphaX * alphaX) +
                Sin2Phi(wh) / (alphaY * alphaY))) /
                (Pi * alphaX * alphaY * cos4Theta);
        }
        Vector3f Sample_wh(const Vector3f& wo, const Point2f& u) const;
    private:
        float Lambda(const Vector3f& w) const
        {
            float absTanTheta = std::abs(TanTheta(w));
            if (std::isinf(absTanTheta)) return 0;
            float alpha = std::sqrt(Cos2Phi(w) * alphaX * alphaX +
                Sin2Phi(w) * alphaY * alphaY);
            float a = 1 / (alpha * absTanTheta);
            if (a >= 1.6f)
                return 0;
            return (1 - 1.259f * a + 0.396f * a * a) /
                (3.535f * a + 2.181f * a * a);
        }
        const float alphaX, alphaY;
    };

    class TrowbridgeReitzDistribution : public MicrofacetDistribution
    {
    public:
        TrowbridgeReitzDistribution(float alphaX, float alphaY,
            bool samplevis = true)
            : MicrofacetDistribution(samplevis), alphaX(alphaX), alphaY(alphaY) {
        }
        inline static float TrowbridgeReitzDistribution::RoughnessToAlpha(float roughness) {
            roughness = std::max(roughness, (float)1e-3);
            float x = std::log(roughness);
            return 1.62142f + 0.819955f * x + 0.1734f * x * x +
                0.0171201f * x * x * x + 0.000640711f * x * x * x * x;
        }
        float D(const Vector3f& wh) const override
        {
            float tan2Theta = Tan2Theta(wh);
            if (std::isinf(tan2Theta)) return 0.;
            const float cos4Theta = Cos2Theta(wh) * Cos2Theta(wh);
            float e = (Cos2Phi(wh) / (alphaX * alphaX) +
                Sin2Phi(wh) / (alphaY * alphaY)) * tan2Theta;
            return 1 / (Pi * alphaX * alphaY * cos4Theta * (1 + e) * (1 + e));
        }
        Vector3f Sample_wh(const Vector3f& wo, const Point2f& u) const override;
        float G1(const Vector3f& w) const
        {
            return 1 / (1 + Lambda(w));
        }
    private:
        float Lambda(const Vector3f& w) const override
        {
            float absTanTheta = std::abs(TanTheta(w));
            if (std::isinf(absTanTheta)) return 0.;
            float alpha = std::sqrt(Cos2Phi(w) * alphaX * alphaX +
                    Sin2Phi(w) * alphaY * alphaY);

            float alpha2Tan2Theta = (alpha * absTanTheta) * (alpha * absTanTheta);
            return (-1 + std::sqrt(1.f + alpha2Tan2Theta)) / 2;
        }
        const float alphaX, alphaY;
    };
}

#endif