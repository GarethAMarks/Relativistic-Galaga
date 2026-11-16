#pragma once
#include "Enemy.h"

// Basic enemy: drifts straight down, fires a single bolt straight down.
class Drone : public Enemy {
public:
    static constexpr float SPEED         = 40.0f;  // px/s downward
    static constexpr float FIRE_INTERVAL = 2.0f;   // seconds between shots
    static constexpr int   SCORE         = 20;

    Drone(float spawnX, float labTime);

    EnemyKind kind()       const override { return EnemyKind::Drone; }
    int       scoreValue() const override { return SCORE; }
    void      tick(float dt, float labTime, std::vector<EnemyShot>& shots) override;

    static constexpr float BOLT_SPEED = 250.0f;

private:
    float fireCooldown_;
};
