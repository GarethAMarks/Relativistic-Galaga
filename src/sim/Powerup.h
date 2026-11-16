#pragma once
#include "relativity/Vec2.h"
#include <raylib.h>

class Simulation;  // forward declaration — avoids circular include

enum class PowerupKind { Boost, Heal, Twin };

class Powerup {
public:
    static constexpr float SPEED  = 60.0f;   // px/s downward
    static constexpr float RADIUS = 13.0f;   // display (max half-size) and pickup radius

    Vec2 pos;
    Vec2 vel   = {0.0f, SPEED};
    bool alive = true;

    explicit Powerup(float spawnX);
    virtual ~Powerup() = default;

    void tick(float dt);

    // Base tint: multiplies the sprite, and is the colour that gets
    // Doppler-shifted in Lorentzian mode.
    virtual Color       color()              const = 0;
    virtual PowerupKind kind()               const = 0;
    virtual void        activate(Simulation& sim)  = 0;
};
