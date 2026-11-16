#include "BoostFrame.h"
#include <cmath>
#include <algorithm>

BoostFrame BoostFrame::fromVelocity(Vec2 vel, float c) {
    BoostFrame f;
    f.v = vel;
    f.c = c;
    float speed = vel.length();
    f.beta  = std::min(speed / c, 0.9999f);
    f.gamma = 1.0f / std::sqrt(1.0f - f.beta * f.beta);
    // Default direction: "forward" (up-screen) when nearly stationary
    f.n = speed > 1e-6f ? vel.normalized() : Vec2{0.0f, -1.0f};
    return f;
}
