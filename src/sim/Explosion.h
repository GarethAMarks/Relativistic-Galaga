#pragma once
#include "relativity/Vec2.h"

struct Explosion {
    Vec2     pos;
    float    age   = 0.0f;
    bool     alive = true;
    float    scale = 1.0f;   // multiplies MAX_RADIUS (bigger for heavies)
    unsigned seed  = 0;      // drives the spark pattern (stable per explosion)

    static constexpr float DURATION   = 0.9f;
    static constexpr float MAX_RADIUS = 40.0f;
};
