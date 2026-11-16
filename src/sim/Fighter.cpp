#include "Fighter.h"
#include <cmath>

static constexpr float TWO_PI = 6.28318530718f;

Fighter::Fighter(float centreX, float phase, float labTime)
    : centreX_(centreX), phase_(phase), spawnTime_(labTime),
      y0_(-HALF_H - 1.0f), fireCooldown_(1.0f)
{
    hp      = MAX_HP;
    pos     = posAt(labTime);
    vel     = velAt(labTime);
    posTime = labTime;
}

Vec2 Fighter::posAt(float t) const {
    float s = t - spawnTime_;
    float w = TWO_PI / PERIOD;
    return { centreX_ + AMPLITUDE * std::sin(w * s + phase_),
             y0_ + SPEED_Y * s };
}

Vec2 Fighter::velAt(float t) const {
    float s = t - spawnTime_;
    float w = TWO_PI / PERIOD;
    return { AMPLITUDE * w * std::cos(w * s + phase_), SPEED_Y };
}

void Fighter::tick(float dt, float labTime, std::vector<EnemyShot>& shots) {
    if (!alive) return;
    pos     = posAt(labTime);
    vel     = velAt(labTime);
    posTime = labTime;
    if (hitFlash > 0.0f) hitFlash -= dt;
    if (pos.y > 630.0f) {
        alive = false;
        return;
    }

    fireCooldown_ -= dt;
    if (fireCooldown_ <= 0.0f) {
        if (burstLeft_ == 0) burstLeft_ = BURST_SHOTS;
        Vec2 boltVel = {0.0f, BOLT_SPEED};
        float gy = pos.y + HALF_H * 0.6f;
        shots.push_back({ {pos.x - GUN_OFFSET_X, gy}, boltVel, false });
        shots.push_back({ {pos.x + GUN_OFFSET_X, gy}, boltVel, false });
        --burstLeft_;
        fireCooldown_ += (burstLeft_ > 0) ? BURST_SPACING : VOLLEY_GAP;
    }
}
