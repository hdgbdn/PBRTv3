#include "microfacet.h"

namespace pbrt
{
    static void BeckmannSample11(float cosThetaI, float U1, float U2,
        float* slope_x, float* slope_y) {
        /* Special case (normal incidence) */
        if (cosThetaI > .9999) {
            float r = std::sqrt(-std::log(1.0f - U1));
            float sinPhi = std::sin(2 * Pi * U2);
            float cosPhi = std::cos(2 * Pi * U2);
            *slope_x = r * cosPhi;
            *slope_y = r * sinPhi;
            return;
        }

        /* The original inversion routine from the paper contained
           discontinuities, which causes issues for QMC integration
           and techniques like Kelemen-style MLT. The following code
           performs a numerical inversion with better behavior */
        float sinThetaI =
            std::sqrt(std::max((float)0, (float)1 - cosThetaI * cosThetaI));
        float tanThetaI = sinThetaI / cosThetaI;
        float cotThetaI = 1 / tanThetaI;

        /* Search interval -- everything is parameterized
           in the Erf() domain */
        float a = -1, c = Erf(cotThetaI);
        float sample_x = std::max(U1, (float)1e-6f);

        /* Start with a good initial guess */
        // float b = (1-sample_x) * a + sample_x * c;

        /* We can do better (inverse of an approximation computed in
         * Mathematica) */
        float thetaI = std::acos(cosThetaI);
        float fit = 1 + thetaI * (-0.876f + thetaI * (0.4265f - 0.0594f * thetaI));
        float b = c - (1 + c) * std::pow(1 - sample_x, fit);

        /* Normalization factor for the CDF */
        static const float SQRT_PI_INV = 1.f / std::sqrt(Pi);
        float normalization =
            1 /
            (1 + c + SQRT_PI_INV * tanThetaI * std::exp(-cotThetaI * cotThetaI));

        int it = 0;
        while (++it < 10) {
            /* Bisection criterion -- the oddly-looking
               Boolean expression are intentional to check
               for NaNs at little additional cost */
            if (!(b >= a && b <= c)) b = 0.5f * (a + c);

            /* Evaluate the CDF and its derivative
               (i.e. the density function) */
            float invErf = ErfInv(b);
            float value =
                normalization *
                (1 + b + SQRT_PI_INV * tanThetaI * std::exp(-invErf * invErf)) -
                sample_x;
            float derivative = normalization * (1 - invErf * tanThetaI);

            if (std::abs(value) < 1e-5f) break;

            /* Update bisection intervals */
            if (value > 0)
                c = b;
            else
                a = b;

            b -= value / derivative;
        }

        /* Now convert back into a slope value */
        *slope_x = ErfInv(b);

        /* Simulate Y component */
        *slope_y = ErfInv(2.0f * std::max(U2, (float)1e-6f) - 1.0f);
    }
    static Vector3f BeckmannSample(const Vector3f& wi, float alpha_x, float alpha_y,
        float U1, float U2) {
        // 1. stretch wi
        Vector3f wiStretched =
            Normalize(Vector3f(alpha_x * wi.x, alpha_y * wi.y, wi.z));

        // 2. simulate P22_{wi}(x_slope, y_slope, 1, 1)
        float slope_x, slope_y;
        BeckmannSample11(CosTheta(wiStretched), U1, U2, &slope_x, &slope_y);

        // 3. rotate
        float tmp = CosPhi(wiStretched) * slope_x - SinPhi(wiStretched) * slope_y;
        slope_y = SinPhi(wiStretched) * slope_x + CosPhi(wiStretched) * slope_y;
        slope_x = tmp;

        // 4. unstretch
        slope_x = alpha_x * slope_x;
        slope_y = alpha_y * slope_y;

        // 5. compute normal
        return Normalize(Vector3f(-slope_x, -slope_y, 1.f));
    }

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

    Vector3f TrowbridgeReitzDistribution::Sample_wh(const Vector3f &wo, const Point2f &u) const
    {
        // TODO need implement
        return pbrt::Vector3f();
    }
}