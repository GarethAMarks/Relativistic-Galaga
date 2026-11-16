#pragma once
#include "relativity/Vec2.h"

struct Bolt {
    Vec2  pos;
    Vec2  vel;
    bool  alive     = true;
    float spawnTime = 0.0f;   // lab time at creation
    float doppler   = 1.0f;   // cached at spawn (used in M5 for color tint)
    float hw        = HW;     // per-bolt half-extents (heavy bolts are larger)
    float hh        = HH;
    bool  big       = false;  // heavy-cannon bolt

    // Default half-extents for AABB collision (draw rect 6x14)
    static constexpr float HW = 3.0f;
    static constexpr float HH = 7.0f;
    // Heavy-cannon bolt half-extents
    static constexpr float BIG_HW = 5.0f;
    static constexpr float BIG_HH = 11.0f;
};
