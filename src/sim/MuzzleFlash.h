#pragma once
#include "relativity/Vec2.h"
#include <raylib.h>

struct MuzzleFlash {
    Vec2  pos;
    Vec2  vel;          // updated each frame from the firing entity
    float age      = 0.0f;
    bool  alive    = true;
    bool  trackShip = false;  // if true, vel is synced from ship each tick
    Color color;   // base color; alpha is computed from age at draw time

    static constexpr float DURATION = 0.16f;
    static constexpr float RADIUS_H = 3.5f;   // horizontal semi-axis
    static constexpr float RADIUS_V = 7.0f;   // vertical semi-axis
};
