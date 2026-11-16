#include "ColorRemap.h"
#include <cmath>

namespace render {

Color hueShiftFromDoppler(Color base, float dopplerFactor, float k) {
    Vector3 hsv = ColorToHSV(base);
    // h_render = h_0 + k * ln(D), wrapped to [0, 360)
    float shift = k * std::log(dopplerFactor);
    hsv.x = std::fmod(hsv.x + shift + 720.0f, 360.0f);
    return ColorFromHSV(hsv.x, hsv.y, hsv.z);
}

} // namespace render
