#include "Heavy.h"
#include <cmath>

Heavy::Heavy(float spawnX, float labTime)
    : Enemy({spawnX, -HALF_H - 1.0f}),
      fireCooldown_(2.0f)
{
    hp      = MAX_HP;
    vel     = {0.0f, SPEED};
    posTime = labTime;
}

void Heavy::tick(float dt, float labTime, std::vector<EnemyShot>& shots) {
    if (!alive) return;
    pos.y  += SPEED * dt;
    posTime = labTime;
    if (hitFlash > 0.0f) hitFlash -= dt;
    if (pos.y > 640.0f) {
        alive = false;
        return;
    }
    fireCooldown_ -= dt;
    if (fireCooldown_ <= 0.0f) {
        fireCooldown_ += FIRE_INTERVAL;
        Vec2  muzzle = {pos.x, pos.y + HALF_H * 0.8f};
        float a      = SPREAD_DEG * 3.14159265f / 180.0f;
        const float angles[3] = { -a, 0.0f, +a };
        for (float th : angles) {
            Vec2 v = { BOLT_SPEED * std::sin(th), BOLT_SPEED * std::cos(th) };
            shots.push_back({ muzzle, v, true });
        }
    }
}
