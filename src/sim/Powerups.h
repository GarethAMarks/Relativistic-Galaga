#pragma once
#include "Powerup.h"

// Raises the ship's maximum Lorentz factor.
class BoostPowerup : public Powerup {
public:
    static constexpr float GAMMA_DELTA = 0.05f;
    static constexpr Color COLOR = {110, 200, 255, 255};
    explicit BoostPowerup(float spawnX) : Powerup(spawnX) {}
    Color       color() const override { return COLOR; }
    PowerupKind kind()  const override { return PowerupKind::Boost; }
    void        activate(Simulation& sim) override;
};

// Restores one point of hull.
class HealPowerup : public Powerup {
public:
    static constexpr int   HP    = 1;
    static constexpr Color COLOR = {255, 112, 112, 255};
    explicit HealPowerup(float spawnX) : Powerup(spawnX) {}
    Color       color() const override { return COLOR; }
    PowerupKind kind()  const override { return PowerupKind::Heal; }
    void        activate(Simulation& sim) override;
};

// Twin cannons: fire two parallel bolts for DURATION seconds of ship proper time.
class TwinShotPowerup : public Powerup {
public:
    static constexpr float DURATION = 10.0f;
    static constexpr Color COLOR = {253, 249, 0, 255};
    explicit TwinShotPowerup(float spawnX) : Powerup(spawnX) {}
    Color       color() const override { return COLOR; }
    PowerupKind kind()  const override { return PowerupKind::Twin; }
    void        activate(Simulation& sim) override;
};
