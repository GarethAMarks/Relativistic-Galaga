#pragma once
#include "relativity/Vec2.h"

struct Ship {
    Vec2 pos = {400.0f, 530.0f};
    Vec2 vel = {0.0f, 0.0f};

    // Half-extents for AABB collision with enemy projectiles.
    static constexpr float HW = 8.0f;
    static constexpr float HH = 13.0f;

    // Per-axis gamma cap; each Boost powerup increases this.
    float gammaMax  = 1.0328f;

    // Base acceleration (px/s^2) used to set timeToMax at reset.
    static constexpr float BASE_ACCEL = 600.0f;

    // Set by Simulation::reset so that accel = vmax / timeToMax at any gammaMax.
    // Keeps time-to-max-speed constant as gammaMax grows from boost pickups.
    float timeToMax = 0.25f;

    void update(float dt, float c_game);
    void reset();
};
