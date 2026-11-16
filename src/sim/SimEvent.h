#pragma once
#include "relativity/Vec2.h"
#include "Enemy.h"

// Things that happened during one Simulation::update(), for presentation
// layers (audio, and anything else that wants to react) to consume.  The
// simulation itself never plays sounds.
enum class SimEventType {
    PlayerFired,
    EnemyFired,        // one per enemy volley (not per bolt)
    EnemyHit,          // non-lethal hit on an enemy
    EnemyDestroyed,
    PlayerHit,         // non-lethal hit on the player
    PlayerDestroyed,
    PowerupCollected,
};

struct SimEvent {
    SimEventType type;
    Vec2         pos;                         // lab-frame position
    EnemyKind    enemy = EnemyKind::Drone;    // for enemy events
};
