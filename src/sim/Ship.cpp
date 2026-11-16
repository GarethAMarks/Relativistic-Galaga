#include "Ship.h"
#include <raylib.h>
#include <cmath>
#include <algorithm>

// Returns the applied force per unit mass for one axis.
// Provides friction (opposing vel) when no key is held, clamped so it can't
// overshoot zero in a single timestep.
static float axisForce(float v, bool posKey, bool negKey, float a0, float dt) {
    if (posKey && !negKey) return  a0;
    if (negKey && !posKey) return -a0;
    float friction = std::min(a0, std::abs(v) / dt);
    if (v > 0.0f) return -friction;
    if (v < 0.0f) return  friction;
    return 0.0f;
}

void Ship::update(float dt, float c_game) {
    float vmax   = c_game * std::sqrt(1.0f - 1.0f / (gammaMax * gammaMax));
    float accel0 = vmax / timeToMax;

    float speed = vel.length();
    float gamma = (speed > 1e-6f)
                ? 1.0f / std::sqrt(1.0f - (speed / c_game) * (speed / c_game))
                : 1.0f;

    Vec2 F = {
        axisForce(vel.x, IsKeyDown(KEY_RIGHT), IsKeyDown(KEY_LEFT), accel0, dt),
        axisForce(vel.y, IsKeyDown(KEY_DOWN),  IsKeyDown(KEY_UP),   accel0, dt)
    };

    // Relativistic equation of motion: dp/dt = F, p = γmv
    // → a = F/γ − (F·v)v / (γ²c²)
    // Longitudinal (along v) suppressed by 1/γ³, transverse by 1/γ.
    Vec2 a = F / gamma;
    if (speed > 1e-6f)
        a = a - vel * (F.dot(vel) / (gamma * gamma * c_game * c_game));

    vel = vel + a * dt;

    // Clamp total speed to vmax so diagonal motion can't exceed the gamma cap.
    float newSpeed = vel.length();
    if (newSpeed > vmax)
        vel = vel * (vmax / newSpeed);

    pos = pos + vel * dt;

    if (pos.x < 0.0f)    pos.x += 800.0f;
    if (pos.x >= 800.0f) pos.x -= 800.0f;
    pos.y = std::max(30.0f, std::min(570.0f, pos.y));
}

void Ship::reset() {
    pos = {400.0f, 530.0f};
    vel = {0.0f, 0.0f};
}
