// "How to Play" deck: controls and powerups.
#include "InfoPanels.h"
#include "render/DrawUtil.h"
#include "sim/Powerups.h"
#include <raylib.h>
#include <initializer_list>

using namespace panels;
using namespace render;

namespace {

// ============================================================================
// Panel 1: controls
// ============================================================================

void controlsA(const Fig& f) {   // arrow keys + space, next to the ship they drive
    const float k = 36.0f, kh = 32.0f;
    Vec2 K = { f.c.x - 85.0f, f.c.y - 25.0f };          // centre of the down key
    auto keyAt = [&](Vec2 c, Vec2 dir) {
        arrowKey({ c.x - k / 2.0f, c.y - kh / 2.0f, k, kh }, dir);
    };
    keyAt({ K.x,         K.y - 38.0f }, {  0.0f, -1.0f });
    keyAt({ K.x - 40.0f, K.y },         { -1.0f,  0.0f });
    keyAt({ K.x,         K.y },         {  0.0f,  1.0f });
    keyAt({ K.x + 40.0f, K.y },         {  1.0f,  0.0f });
    textCentred("move", K.x, K.y + 24.0f, 16, ARROW_COL);

    keycap({ K.x - 75.0f, f.c.y + 50.0f, 150.0f, 30.0f }, "SPACE", 16);
    textCentred("fire", K.x, f.c.y + 86.0f, 16, BOLT_COL);

    Vec2 s = { f.c.x + 95.0f, f.c.y + 30.0f };
    ship(f, s);
    arrow({ s.x - 18.0f, s.y }, { s.x - 52.0f, s.y }, ARROW_COL, 2.0f);
    arrow({ s.x + 18.0f, s.y }, { s.x + 52.0f, s.y }, ARROW_COL, 2.0f);
    arrow({ s.x, s.y + 30.0f }, { s.x, s.y + 60.0f }, ARROW_COL, 2.0f);
    arrow({ s.x - 26.0f, s.y - 14.0f }, { s.x - 26.0f, s.y - 44.0f }, ARROW_COL, 2.0f);
    bolt(f, { s.x, s.y - 70.0f }, { 0.0f, -1.0f });
    bolt(f, { s.x, s.y - 125.0f }, { 0.0f, -1.0f });
}

void controlsB(const Fig& f) {   // the other keys
    const float x0 = f.box.x + 28.0f, tx = f.box.x + 120.0f;
    const float ys[3] = { f.c.y - 62.0f, f.c.y, f.c.y + 62.0f };

    keycap({ x0,         ys[0] - 16.0f, 32.0f, 32.0f }, "[", 20);
    keycap({ x0 + 40.0f, ys[0] - 16.0f, 32.0f, 32.0f }, "]", 20);
    text("lower / raise the",   tx, ys[0] - 18.0f, 18, LABEL_COL);
    text("speed of light, c",   tx, ys[0] + 2.0f,  18, LABEL_COL);

    keycap({ x0, ys[1] - 16.0f, 72.0f, 32.0f }, "M", 18);
    text("music on / off", tx, ys[1] - 9.0f, 18, LABEL_COL);

    keycap({ x0, ys[2] - 16.0f, 72.0f, 32.0f }, "Esc", 18);
    text("back to the menu", tx, ys[2] - 9.0f, 18, LABEL_COL);
}

// ============================================================================
// Panel 2: powerups
// ============================================================================

void powerupsA(const Fig& f) {   // the three powerups and what they do
    // Note: raylib's TextFormat rotates through 4 static buffers, so the three
    // strings below stay valid until they are drawn.
    struct Row { const Texture2D& tex; Color col; const char* name; const char* what; };
    const Row rows[3] = {
        { f.a.powerupBoost, BoostPowerup::COLOR, "Boost",
          TextFormat("raises gamma_max by %.2f", BoostPowerup::GAMMA_DELTA) },
        { f.a.powerupHeal, HealPowerup::COLOR, "Repair",
          TextFormat("restores %d point of hull", HealPowerup::HP) },
        { f.a.powerupTwin, TwinShotPowerup::COLOR, "Twin cannons",
          TextFormat("two bolts per shot for %.0f s", TwinShotPowerup::DURATION) },
    };
    for (int i = 0; i < 3; ++i) {
        float y = f.c.y - 62.0f + 62.0f * i;
        spriteFit(rows[i].tex, { f.box.x + 48.0f, y }, 18.0f, rows[i].col);
        text(rows[i].name, f.box.x + 84.0f, y - 19.0f, 20, rows[i].col);
        text(rows[i].what, f.box.x + 84.0f, y + 4.0f,  16, LABEL_COL);
    }
}

void powerupsB(const Fig& f) {   // twin cannons in action, with the HUD timer
    Vec2 s = { f.c.x, f.c.y + 55.0f };
    ship(f, s);
    for (float dy : { 58.0f, 100.0f, 142.0f })
        for (float dx : { -7.0f, 7.0f })   // matches the game's twin-bolt spacing
            bolt(f, { s.x + dx, s.y - dy }, { 0.0f, -1.0f });

    // Mock of the in-game HUD readout.
    float hx = f.box.x + 14.0f, hy = f.box.y + 40.0f;
    text("TWIN CANNONS 7.3 s", hx, hy, 14, YELLOW);
    DrawRectangle((int)hx, (int)hy + 18, 120, 4, Color{ 60, 60, 20, 255 });
    DrawRectangle((int)hx, (int)hy + 18, (int)(120 * 0.73f), 4, YELLOW);

    footnote(f, TextFormat("lasts %.0f s on your ship's clock (tau_ship)",
                           TwinShotPowerup::DURATION), 14, LABEL_COL);
}

// ============================================================================
// Panel table
// ============================================================================

const Panel PANELS[] = {
    { "Controls",
      "Fly and fire", controlsA, "Other keys", controlsB,
      "Fly your ship with the arrow keys and press Space to fire. Your top speed is set "
      "by your ship's maximum Lorentz factor, gamma_max, shown at the top left of the "
      "screen. The [ and ] keys lower or raise the speed of light, c. M switches the "
      "music on or off, and Esc returns to the menu." },

    { "Powerups",
      "Fly into a powerup to collect it", powerupsA, "Twin cannons", powerupsB,
      "Powerups drift down the screen: fly into one to collect it. Boost raises your "
      "ship's maximum Lorentz factor, gamma_max, so you can fly closer to the speed of "
      "light, where the relativistic effects are stronger. Repair restores one point of "
      "hull: you can take three hits before your ship is destroyed. Twin cannons fire two "
      "parallel bolts with every shot for 10 seconds, timed on your ship's own clock, "
      "tau_ship, so at high speed they last longer in lab time." },
};

} // namespace

const Deck& howToPlayDeck() {
    static const Deck deck{ "HOW TO PLAY", PANELS, (int)(sizeof(PANELS) / sizeof(PANELS[0])) };
    return deck;
}
