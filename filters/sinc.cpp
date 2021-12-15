#include "sinc.h"

namespace pbrt {

    // Sinc Filter Method Definitions
    float LanczosSincFilter::Evaluate(const Point2f& p) const {
        return WindowedSinc(p.x, radius.x) * WindowedSinc(p.y, radius.y);
    }

}  // namespace pbrt
