#include "Powerup.h"

Powerup::Powerup(float spawnX) : pos({spawnX, -RADIUS - 1.0f}) {}

void Powerup::tick(float dt) {
    if (!alive) return;
    pos.y += SPEED * dt;
    if (pos.y > 620.0f) alive = false;
}
