#pragma once
#include "Enemy.h"
#include <vector>

class Spawner {
public:
    // Replace the enemy list with a fresh grid formation.
    // wave >= 1; higher waves move faster.
    void spawnWave(std::vector<Enemy>& enemies, int wave);
};
