#pragma once
#include "relativity/Vec2.h"
#include <vector>

enum class EnemyKind { Formation, Drone, Fighter, Heavy };

// A shot an enemy wants to fire this tick.  Simulation turns these into Bolts.
struct EnemyShot {
    Vec2 pos;
    Vec2 vel;
    bool big = false;   // heavy-cannon bolt (larger sprite/hitbox)
};

// Base enemy.  Concrete (not abstract) so the legacy formation code can still
// keep a std::vector<Enemy>; the spawned enemy types override the virtuals.
struct Enemy {
    Vec2  pos;
    Vec2  vel      = {0.0f, 0.0f};
    bool  alive    = true;
    int   hp       = 1;
    float hitFlash = 0.0f;   // seconds of white "hit" flash remaining
    float posTime  = 0.0f;   // lab time at which pos/vel are valid

    // Default half-extents for AABB collision (formation enemies and drones).
    static constexpr float HW = 15.0f;
    static constexpr float HH = 10.0f;
    static constexpr float HIT_FLASH_TIME = 0.12f;

    Enemy() = default;
    explicit Enemy(Vec2 spawnPos) : pos(spawnPos) {}
    virtual ~Enemy() = default;

    virtual EnemyKind kind()       const { return EnemyKind::Formation; }
    virtual float     halfW()      const { return HW; }
    virtual float     halfH()      const { return HH; }
    virtual int       scoreValue() const { return 10; }

    // Lab-frame trajectory, valid for times near posTime.  Used by the
    // Lorentzian renderer to solve the ship-frame simultaneity condition for
    // objects that are not in uniform motion (e.g. the weaving fighter).
    virtual Vec2 posAt(float t) const { return pos + vel * (t - posTime); }
    virtual Vec2 velAt(float t) const { (void)t; return vel; }

    // Advance to lab time `labTime`; append any shots fired to `shots`.
    virtual void tick(float dt, float labTime, std::vector<EnemyShot>& shots) {
        (void)dt; (void)labTime; (void)shots;
    }

    // Apply one point of damage.  Returns true if this destroyed the enemy.
    bool takeHit() {
        hitFlash = HIT_FLASH_TIME;
        if (--hp <= 0) { alive = false; return true; }
        return false;
    }
};
