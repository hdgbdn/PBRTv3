#include "microfacet.h"

namespace pbrt
{
    Vector3f BeckmannDistribution::Sample_wh(const Vector3f& wo, const Point2f& u) const
    {
        if(!sampleVisibleArea)
        {
            float tan2Theta, phi;
            if( alphaX == alphaY)
            {
                float logSample = std::log(1 - u[0]);
                if (std::isinf(logSample)) logSample = 0;
                tan2Theta = -alphaX * alphaX * logSample;
                phi = u[1] * Pi * 2;
            }
            else
            {
                float logSample = std::log(u[0]);
                phi = std::atan(alphaY / alphaX *
                    std::tan(2 * Pi * u[1] + 0.5f * Pi));
                if (u[1] > 0.5f)
                    phi += Pi;
                float sinPhi = std::sin(phi), cosPhi = std::cos(phi);
                float alphax2 = alphaX * alphaX, alphay2 = alphaY * alphaY;
                tan2Theta = -logSample /
                    (cosPhi * cosPhi / alphax2 + sinPhi * sinPhi / alphay2);
            }
            float cosTheta = 1 / std::sqrt(1 + tan2Theta);
            float sinTheta = std::sqrt(std::max((float)0, 1 - cosTheta * cosTheta));
            Vector3f wh = SphericalDirection(sinTheta, cosTheta, phi);
            if (!SameHemisphere(wo, wh)) wh = -wh;
            return wh;
        }
        else
        {
            Vector3f wh;
            bool flip = wo.z < 0;
            wh = BeckmannSample(flip ? -wo : wo, alphaX, alphaY, u[0], u[1]);
            if (flip) wh = -wh;
            return wh;
        }
    }
    float MicrofacetDistribution::Pdf(const Vector3f& wo, const Vector3f& wh) const
    {
        if (sampleVisibleArea)
            return D(wh) * G1(wo) * AbsDot(wo, wh);
        else
            return D(wh) * AbsCosTheta(wh);
    }
}