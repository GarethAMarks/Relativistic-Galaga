#pragma once
#include "relativity/Vec2.h"

// Short-lived impact flash where a bolt strikes a ship.
struct HitEffect {
    Vec2  pos;              // lab-frame position (valid at current lab time)
    Vec2  vel;              // lab-frame velocity of the thing that was hit
    Vec2  offset;           // onPlayer only: offset from the ship centre
    float age      = 0.0f;
    float rotation = 0.0f;  // degrees, randomised per effect for variety
    bool  alive    = true;
    bool  onPlayer = false; // true: enemy bolt hit the player (rides with ship)

    static constexpr float DURATION    = 0.28f;
    static constexpr float SIZE_ENEMY  = 16.0f;  // max half-size, px
    static constexpr float SIZE_PLAYER = 20.0f;
};
