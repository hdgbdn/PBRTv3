#ifndef PBRT_FILTERS_SINC_H
#define PBRT_FILTERS_SINC_H

// filters/sinc.h*
#include "core/filter.h"

namespace pbrt {

    // Sinc Filter Declarations
    class LanczosSincFilter : public Filter {
    public:
        // LanczosSincFilter Public Methods
        LanczosSincFilter(const Vector2f& radius, float tau)
            : Filter(radius), tau(tau) {}
        float Evaluate(const Point2f& p) const;
        float Sinc(float x) const {
            x = std::abs(x);
            if (x < 1e-5) return 1;
            return std::sin(Pi * x) / (Pi * x);
        }
        float WindowedSinc(float x, float radius) const {
            x = std::abs(x);
            if (x > radius) return 0;
            float lanczos = Sinc(x / tau);
            return Sinc(x) * lanczos;
        }

    private:
        const float tau;
    };

}  // namespace pbrt

#endif  // PBRT_FILTERS_SINC_H
