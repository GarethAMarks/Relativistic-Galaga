#include "Spawner.h"

void Spawner::spawnWave(std::vector<Enemy>& enemies, int wave) {
    enemies.clear();
    // 5 columns x 4 rows, centred in the arena
    const int   cols   = 5;
    const int   rows   = 4;
    const float startX = 175.0f;
    const float startY = 80.0f;
    const float spacX  = 100.0f;
    const float spacY  = 55.0f;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            Enemy e;
            e.pos  = { startX + c * spacX, startY + r * spacY };
            e.alive = true;
            enemies.push_back(e);
        }
    }
    (void)wave;  // placeholder: future waves could vary spacing/speed
}
