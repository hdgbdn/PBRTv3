#include "sinc.h"
#include "core/paramset.h"

namespace pbrt {

    // Sinc Filter Method Definitions
    float LanczosSincFilter::Evaluate(const Point2f& p) const {
        return WindowedSinc(p.x, radius.x) * WindowedSinc(p.y, radius.y);
    }

    LanczosSincFilter* CreateLanczosSincFilter(const ParamSet& ps)
    {
        float xw = ps.FindOneFloat("xwidth", 0.5f);
        float yw = ps.FindOneFloat("ywidth", 0.5f);
        float tau = ps.FindOneFloat("tau", 3.f);
        return new LanczosSincFilter(Vector2f(xw, yw), tau);
    }
}  // namespace pbrt
