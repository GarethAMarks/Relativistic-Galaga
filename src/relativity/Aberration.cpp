#include "Aberration.h"
#include <cmath>
#include <algorithm>

namespace relativity {

// cos(theta') = (cos(theta) - beta) / (1 - beta*cos(theta))   [lab -> ship]
// cos(theta)  = (cos(theta') + beta) / (1 + beta*cos(theta')) [ship -> lab]
//
// The perpendicular direction unit vector is preserved across the transform;
// only the mix of par/perp magnitudes changes.
Vec2 aberrate(Vec2 unitDir, const BoostFrame& f, bool toShip) {
    float cosIn = unitDir.dot(f.n);
    cosIn = std::max(-1.0f, std::min(1.0f, cosIn));

    float cosOut;
    if (toShip) {
        cosOut = (cosIn - f.beta) / (1.0f - f.beta * cosIn);
    } else {
        cosOut = (cosIn + f.beta) / (1.0f + f.beta * cosIn);
    }
    cosOut = std::max(-1.0f, std::min(1.0f, cosOut));

    float sinIn = std::sqrt(std::max(0.0f, 1.0f - cosIn * cosIn));
    if (sinIn < 1e-7f) {
        // Direction is (anti-)parallel to n; result is ±n
        return cosOut >= 0.0f ? f.n : Vec2{-f.n.x, -f.n.y};
    }

    // Unit vector perpendicular to n in the plane of unitDir and n
    Vec2 perpHat = (unitDir - f.n * cosIn) * (1.0f / sinIn);
    float sinOut = std::sqrt(std::max(0.0f, 1.0f - cosOut * cosOut));
    return f.n * cosOut + perpHat * sinOut;
}

} // namespace relativity
