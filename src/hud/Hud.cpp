#include "Hud.h"
#include "sim/Simulation.h"
#include "sim/Powerups.h"
#include <raylib.h>
#include <algorithm>

void Hud::draw(const Simulation& sim,
               const BoostFrame& boost,
               bool  lorentzianMode,
               float c_game) const {
    // ---- Top-left: boost state ----
    DrawText(TextFormat("beta:    %.4f", boost.beta),  10, 10, 18, GREEN);
    DrawText(TextFormat("gamma:   %.4f", boost.gamma), 10, 32, 18, GREEN);
    DrawText(TextFormat("gamma_max: %.3f", sim.ship().gammaMax), 10, 54, 16, DARKGREEN);
    DrawText(TextFormat("c_game:  %.0f px/s", c_game), 10, 74, 16, DARKGREEN);
    DrawText(TextFormat("Mode: %s", lorentzianMode ? "LORENTZIAN" : "Newtonian"),
             10, 94, 16, lorentzianMode ? YELLOW : SKYBLUE);

    // ---- Top-right: dual clocks + threat ----
    DrawText("t_lab:",  530, 10, 18, WHITE);
    DrawText(TextFormat("%.3f s", sim.labTime()), 628, 10, 18, WHITE);
    DrawText("tau_ship:", 530, 32, 18, ORANGE);
    DrawText(TextFormat("%.3f s", sim.shipProperTime()), 628, 32, 18, ORANGE);
    DrawText(TextFormat("Threat level %d", sim.threatLevel()), 530, 56, 16, Color{255, 120, 120, 255});

    // Twin-cannon timer (counts down on the ship's own clock).
    float twin = sim.twinShotTime();
    if (twin > 0.0f) {
        DrawText(TextFormat("TWIN CANNONS %.1f s", twin), 530, 78, 16, YELLOW);
        float frac = std::clamp(twin / TwinShotPowerup::DURATION, 0.0f, 1.0f);
        DrawRectangle(530, 97, 200, 4, Color{60, 60, 20, 255});
        DrawRectangle(530, 97, (int)(200 * frac), 4, YELLOW);
        DrawText("(ship time)", 700, 80, 10, GRAY);
    }

    // ---- Bottom bar ----
    DrawRectangle(0, 578, 800, 22, ColorAlpha(BLACK, 0.7f));
    DrawText(TextFormat("Score: %d", sim.score()), 10, 581, 16, WHITE);
    DrawText("arrows: move  |  space: fire  |  [/]: c  |  M: music", 150, 581, 14, DARKGRAY);

    // Health: coloured squares (red = full, dark = lost)
    DrawText("HP", 698, 581, 14, RED);
    for (int i = 0; i < Simulation::PLAYER_MAX_HEALTH; ++i) {
        Color sq = (i < sim.health()) ? RED : Color{55, 20, 20, 255};
        DrawRectangle(722 + i * 18, 583, 12, 11, sq);
    }
}
