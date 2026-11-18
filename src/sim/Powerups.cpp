#include "Powerups.h"
#include "Simulation.h"

void BoostPowerup::activate(Simulation& sim) {
    sim.applyBoost(GAMMA_DELTA);
}

void HealPowerup::activate(Simulation& sim) {
    sim.applyHeal(HP);
}

void TwinShotPowerup::activate(Simulation& sim) {
    sim.applyTwinShot(DURATION);
}
