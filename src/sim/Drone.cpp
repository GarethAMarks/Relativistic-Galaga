#include "Drone.h"

Drone::Drone(float spawnX, float labTime)
    : Enemy({spawnX, -HH - 1.0f}),
      fireCooldown_(FIRE_INTERVAL)
{
    vel     = {0.0f, SPEED};
    posTime = labTime;
}

void Drone::tick(float dt, float labTime, std::vector<EnemyShot>& shots) {
    if (!alive) return;
    pos.y  += SPEED * dt;
    posTime = labTime;
    if (hitFlash > 0.0f) hitFlash -= dt;
    if (pos.y > 620.0f) {
        alive = false;
        return;
    }
    fireCooldown_ -= dt;
    if (fireCooldown_ <= 0.0f) {
        fireCooldown_ += FIRE_INTERVAL;
        shots.push_back({ {pos.x, pos.y + HH}, {0.0f, BOLT_SPEED}, false });
    }
}
