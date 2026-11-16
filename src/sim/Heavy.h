#pragma once
#include "Enemy.h"

// Large, slow gunship.  Fires a three-bolt spread of heavy bolts.
class Heavy : public Enemy {
public:
    static constexpr float HALF_W        = 29.0f;
    static constexpr float HALF_H        = 30.0f;
    static constexpr float SPEED         = 22.0f;   // px/s downward
    static constexpr float FIRE_INTERVAL = 2.6f;    // s between spreads
    static constexpr float SPREAD_DEG    = 15.0f;   // angle of side bolts from vertical
    static constexpr float BOLT_SPEED    = 210.0f;
    static constexpr int   MAX_HP        = 5;
    static constexpr int   SCORE         = 5 * 20;  // 5x a drone

    Heavy(float spawnX, float labTime);

    EnemyKind kind()       const override { return EnemyKind::Heavy; }
    float     halfW()      const override { return HALF_W; }
    float     halfH()      const override { return HALF_H; }
    int       scoreValue() const override { return SCORE; }
    void      tick(float dt, float labTime, std::vector<EnemyShot>& shots) override;

private:
    float fireCooldown_;
};
