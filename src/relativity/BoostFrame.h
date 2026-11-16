#pragma once
#include "Vec2.h"

// Parameters of the ship's instantaneous Lorentz boost at a given moment.
struct BoostFrame {
    Vec2  v;      // ship lab-frame velocity (px/s)
    float c;      // in-game speed of light (px/s)
    float beta;   // |v|/c, clamped to [0, 0.9999]
    float gamma;  // 1/sqrt(1-beta^2)
    Vec2  n;      // unit vector v/|v|; defaults to {0,-1} when beta~0

    static BoostFrame fromVelocity(Vec2 vel, float c);
};
