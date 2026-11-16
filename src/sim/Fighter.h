#pragma once
#include "Enemy.h"

// Weaves side-to-side on a sinusoid while descending at a constant rate.
// Fires bursts from both wing guns, producing two parallel lines of bolts.
class Fighter : public Enemy {
public:
    static constexpr float HALF_W        = 20.0f;
    static constexpr float HALF_H        = 16.0f;
    static constexpr float SPEED_Y       = 55.0f;    // px/s downward
    static constexpr float AMPLITUDE     = 90.0f;    // px, horizontal weave
    static constexpr float PERIOD        = 3.2f;     // s, one full weave
    static constexpr float VOLLEY_GAP    = 2.2f;     // s between bursts
    static constexpr int   BURST_SHOTS   = 3;        // shots per gun per burst
    static constexpr float BURST_SPACING = 0.18f;    // s between shots in a burst
    static constexpr float GUN_OFFSET_X  = 13.0f;    // wing gun distance from centre
    static constexpr float BOLT_SPEED    = 260.0f;
    static constexpr int   MAX_HP        = 2;
    static constexpr int   SCORE         = 2 * 20;   // 2x a drone

    // centreX: mid-line of the weave.  phase: initial sinusoid phase (rad).
    Fighter(float centreX, float phase, float labTime);

    EnemyKind kind()       const override { return EnemyKind::Fighter; }
    float     halfW()      const override { return HALF_W; }
    float     halfH()      const override { return HALF_H; }
    int       scoreValue() const override { return SCORE; }

    Vec2 posAt(float t) const override;
    Vec2 velAt(float t) const override;
    void tick(float dt, float labTime, std::vector<EnemyShot>& shots) override;

private:
    float centreX_;
    float phase_;
    float spawnTime_;
    float y0_;
    float fireCooldown_;
    int   burstLeft_ = 0;
};
