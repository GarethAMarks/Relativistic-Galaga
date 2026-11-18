// "The Science" deck: one panel per relativistic effect shown in the game.
// Figures are built from game sprites and the game's own relativity code.
#include "InfoPanels.h"
#include "render/DrawUtil.h"
#include "render/ColorRemap.h"
#include "relativity/Aberration.h"
#include "relativity/Doppler.h"
#include "relativity/Lorentz.h"
#include <raylib.h>
#include <algorithm>
#include <cmath>

using namespace panels;
using namespace render;

namespace {

const Color STAR_COL     = { 220, 230, 255, 255 };   // as in the game's starfield
const Color DOPPLER_BASE = { 60, 220, 90, 255 };     // green: shifts to blue / to red

constexpr float C = 1.0f;   // figures work in units of c

BoostFrame boostOf(Vec2 betaVec) { return BoostFrame::fromVelocity(betaVec * C, C); }

// Deterministic pseudo-random numbers for star fields.
float rnd(unsigned& s) { s = s * 1664525u + 1013904223u; return (s >> 8) / 16777216.0f; }

// ============================================================================
// Panel 1: relative motion (Galilean)
// ============================================================================

const float P1_U = 75.0f, P1_V = 65.0f;

void p1Ship(const Fig& f, bool shipFrame) {
    Vec2 s  = { f.c.x - 70.0f, f.c.y + 62.0f };
    Vec2 dr = { f.c.x + 95.0f, f.c.y - 45.0f };
    Vec2 muzzle = { s.x, s.y - 30.0f };
    ship(f, s);
    drone(f, dr);
    if (shipFrame) {
        textCentred("at rest", s.x, s.y + 32.0f, 16, LABEL_COL);
        arrow({ dr.x - 26.0f, dr.y }, { dr.x - 26.0f - P1_V, dr.y }, ARROW_COL);
        text("v", dr.x - 26.0f - P1_V - 14.0f, dr.y - 8.0f, 18, ARROW_COL);
        arrow({ s.x + 18.0f, muzzle.y }, { s.x + 18.0f, muzzle.y - P1_U }, ARROW_COL);
        text("u", s.x + 26.0f, muzzle.y - P1_U, 18, ARROW_COL);
        bolt(f, { muzzle.x, muzzle.y - 0.6f * P1_U }, { 0.0f, -1.0f });
    } else {
        textCentred("at rest", dr.x, dr.y + 24.0f, 16, LABEL_COL);
        arrow({ s.x + 22.0f, s.y + 14.0f }, { s.x + 22.0f + P1_V, s.y + 14.0f }, ARROW_COL);
        text("v", s.x + 26.0f + P1_V, s.y + 6.0f, 18, ARROW_COL);
        Vec2 tipU  = { muzzle.x, muzzle.y - P1_U };
        Vec2 tipUV = { muzzle.x + P1_V, muzzle.y - P1_U };
        dashed(muzzle, tipU, GUIDE_COL);
        dashed(tipU, tipUV, GUIDE_COL);
        text("u", muzzle.x - 16.0f, muzzle.y - 0.6f * P1_U, 16, GUIDE_COL);
        text("v", muzzle.x + 0.45f * P1_V, tipU.y - 20.0f, 16, GUIDE_COL);
        arrow(muzzle, tipUV, ARROW_COL);
        text("u + v", tipUV.x + 6.0f, tipUV.y - 4.0f, 18, ARROW_COL);
        bolt(f, muzzle + (tipUV - muzzle) * 0.45f, (tipUV - muzzle).normalized());
    }
}

void p1A(const Fig& f) { p1Ship(f, true); }
void p1B(const Fig& f) { p1Ship(f, false); }

// ============================================================================
// Panel 2: constant speed of light
// ============================================================================

void p2Common(const Fig& f, Vec2 s, float v) {
    ship(f, s);
    arrow({ s.x + 22.0f, s.y + 14.0f }, { s.x + 22.0f + v, s.y + 14.0f }, ARROW_COL);
    text("v", s.x + 26.0f + v, s.y + 6.0f, 18, ARROW_COL);
}

void p2A(const Fig& f) {   // Newtonian bullets: speeds add
    const float U = 70.0f, V = 60.0f;
    Vec2 s = { f.c.x - 20.0f, f.c.y + 50.0f };
    Vec2 muzzle = { s.x, s.y - 30.0f };
    dashedCircle(muzzle, U, GUIDE_COL);
    text("speed u", muzzle.x - U - 10.0f - MeasureText("speed u", 16), muzzle.y - 8.0f, 16, GUIDE_COL);
    p2Common(f, s, V);
    Vec2 tip = { muzzle.x + V, muzzle.y - U };
    arrow(muzzle, tip, ARROW_COL);
    text("|u + v|  >  u", tip.x - 10.0f, tip.y - 24.0f, 18, ARROW_COL);
    bolt(f, muzzle + (tip - muzzle) * 0.45f, (tip - muzzle).normalized());
    footnote(f, "the bullet outruns its own muzzle speed", 14, LABEL_COL);
}

void p2B(const Fig& f) {   // light-speed bolts: speed is c, only the direction changes
    const float R = 75.0f;                       // radius drawn for speed c
    const float beta = 0.6f;
    Vec2 s = { f.c.x - 15.0f, f.c.y + 50.0f };
    Vec2 muzzle = { s.x, s.y - 30.0f };
    BoostFrame b   = boostOf({ beta, 0.0f });
    Vec2       dir = relativity::aberrate({ 0.0f, -1.0f }, b, /*toShip=*/false);
    dashedCircle(muzzle, R, GUIDE_COL);
    text("speed c", muzzle.x - R - 10.0f - MeasureText("speed c", 16), muzzle.y - 8.0f, 16, GUIDE_COL);
    p2Common(f, s, beta * R);
    Vec2 tip = muzzle + dir * R;
    arrow(muzzle, tip, ARROW_COL);
    text("c", tip.x + 8.0f, tip.y - 8.0f, 18, ARROW_COL);
    bolt(f, muzzle + dir * (0.5f * R), dir);
    footnote(f, "the bolt still moves at exactly c", 14, LABEL_COL);
}

// ============================================================================
// Panel 3: the game window is your rest frame
// ============================================================================

const float P3_BETA = 0.8f, P3_GRID = 36.0f, P3_SPACING = 100.0f;

void p3Grid(const Fig& f, float xScale) {
    Color g = { 40, 50, 80, 255 };
    float x0 = f.c.x;
    for (int k = -12; k <= 12; ++k) {
        float x = x0 + k * P3_GRID * xScale;
        DrawLineEx({ x, f.box.y }, { x, f.box.y + f.box.height }, 1.0f, g);
    }
    for (float y = f.c.y - 5 * P3_GRID; y < f.box.y + f.box.height; y += P3_GRID)
        DrawLineEx({ f.box.x, y }, { f.box.x + f.box.width, y }, 1.0f, g);
}

void p3A(const Fig& f) {   // lab frame: drones at rest, ship moving
    p3Grid(f, 1.0f);
    for (int k = -1; k <= 1; ++k) drone(f, { f.c.x + k * P3_SPACING, f.c.y - 45.0f });
    Vec2 s = { f.c.x, f.c.y + 65.0f };
    ship(f, s);
    arrow({ s.x + 22.0f, s.y + 10.0f }, { s.x + 90.0f, s.y + 10.0f }, ARROW_COL);
    text("v = 0.8c", s.x + 40.0f, s.y + 16.0f, 16, ARROW_COL);
}

void p3B(const Fig& f) {   // ship frame: everything else moves, contracted and Doppler-shifted
    BoostFrame b = boostOf({ P3_BETA, 0.0f });
    p3Grid(f, 1.0f / b.gamma);
    Vec2 s = { f.c.x, f.c.y + 65.0f };
    for (int k = -1; k <= 1; ++k) {
        Vec2  off = relativity::contractOffset({ k * P3_SPACING, -110.0f }, b);
        Vec2  p   = s + off;
        float D   = relativity::dopplerFactor(b.n.dot(off.normalized()), b);
        drone(f, p, hueShiftFromDoppler(DRONE_COL, D), &b);
    }
    arrow({ f.c.x + 60.0f, f.c.y - 80.0f }, { f.c.x - 20.0f, f.c.y - 80.0f }, ARROW_COL);
    text("v", f.c.x - 36.0f, f.c.y - 88.0f, 18, ARROW_COL);
    ship(f, s);
    textCentred("at rest", s.x, s.y + 30.0f, 16, LABEL_COL);
}

// ============================================================================
// Panel 4: time dilation (light clock)
// ============================================================================

const float P4_L = 110.0f, P4_BETA = 0.6f;

void mirrorPair(Vec2 bottomCentre, float alpha) {
    Color m = withAlpha(LIGHTGRAY, alpha);
    DrawRectangleV({ bottomCentre.x - 28.0f, bottomCentre.y },         { 56.0f, 5.0f }, m);
    DrawRectangleV({ bottomCentre.x - 28.0f, bottomCentre.y - P4_L - 5.0f }, { 56.0f, 5.0f }, m);
}

void p4A(const Fig& f) {   // on board
    float yb = f.c.y + 55.0f;
    Vec2  base = { f.c.x - 30.0f, yb };
    mirrorPair(base, 1.0f);
    dashed({ base.x, yb - 2.0f }, { base.x, yb - P4_L }, GUIDE_COL);
    bolt(f, { base.x, yb - 0.55f * P4_L }, { 0.0f, -1.0f });
    arrow({ base.x - 14.0f, yb - 25.0f }, { base.x - 14.0f, yb - P4_L + 20.0f }, ARROW_COL, 2.0f);
    arrow({ base.x + 14.0f, yb - P4_L + 20.0f }, { base.x + 14.0f, yb - 25.0f }, ARROW_COL, 2.0f);
    text("L", base.x + 34.0f, yb - 0.5f * P4_L - 8.0f, 18, LABEL_COL);
    ship(f, { f.c.x + 90.0f, f.c.y });
    text("clock on", f.c.x + 62.0f, f.c.y + 34.0f, 14, LABEL_COL);
    text("your ship", f.c.x + 60.0f, f.c.y + 50.0f, 14, LABEL_COL);
    footnote(f, "one tick = 2L / c", 18, BOLT_COL);
}

void p4B(const Fig& f) {   // from the lab: the same tick
    BoostFrame b = boostOf({ P4_BETA, 0.0f });
    float shift = P4_BETA * b.gamma * P4_L;      // ship travel during half a tick
    float yb = f.c.y + 55.0f;
    float x0 = f.c.x - shift;
    Vec2  p0 = { x0, yb }, p1 = { x0 + shift, yb - P4_L }, p2 = { x0 + 2 * shift, yb };
    mirrorPair({ x0, yb }, 0.3f);
    mirrorPair({ x0 + shift, yb }, 0.3f);
    mirrorPair({ x0 + 2 * shift, yb }, 1.0f);
    DrawLineEx(rv(p0), rv(p1), 2.0f, withAlpha(BOLT_COL, 0.7f));
    DrawLineEx(rv(p1), rv(p2), 2.0f, withAlpha(BOLT_COL, 0.7f));
    bolt(f, p0 + (p1 - p0) * 0.5f, (p1 - p0).normalized());
    arrow({ x0 + 2 * shift - 40.0f, yb + 22.0f }, { x0 + 2 * shift + 20.0f, yb + 22.0f }, ARROW_COL);
    text("v = 0.6c", x0 + 2 * shift - 110.0f, yb + 14.0f, 16, ARROW_COL);
    footnote(f, TextFormat("one tick = gamma x 2L / c   (gamma = %.2f)", b.gamma), 16, BOLT_COL);
}

// ============================================================================
// Panel 5: length contraction
// ============================================================================

const float P5_SPACING = 75.0f;

void ruler(float x0, float x1, float y, const char* label) {
    DrawLineEx({ x0, y }, { x1, y }, 2.0f, LABEL_COL);
    DrawLineEx({ x0, y - 7 }, { x0, y + 7 }, 2.0f, LABEL_COL);
    DrawLineEx({ x1, y - 7 }, { x1, y + 7 }, 2.0f, LABEL_COL);
    textCentred(label, 0.5f * (x0 + x1), y + 10.0f, 18, LABEL_COL);
}

void p5A(const Fig& f) {
    for (int k = -1; k <= 1; ++k) fighter(f, { f.c.x + k * P5_SPACING, f.c.y - 10.0f });
    ruler(f.c.x - P5_SPACING - 24.0f, f.c.x + P5_SPACING + 24.0f, f.c.y + 35.0f, "L");
    footnote(f, "at rest relative to you", 16, LABEL_COL);
}

void p5B(const Fig& f) {
    BoostFrame b = boostOf({ 0.866f, 0.0f });   // gamma = 2
    for (int k = -1; k <= 1; ++k) {
        Vec2 off = relativity::contractOffset({ k * P5_SPACING, 0.0f }, b);
        fighter(f, { f.c.x + off.x, f.c.y - 10.0f }, &b);
    }
    float half = (P5_SPACING + 24.0f) / b.gamma;
    ruler(f.c.x - half, f.c.x + half, f.c.y + 35.0f, "L / 2");
    arrow({ f.c.x - 50.0f, f.c.y - 60.0f }, { f.c.x + 50.0f, f.c.y - 60.0f }, ARROW_COL);
    textCentred("v = 0.87c", f.c.x, f.c.y - 88.0f, 16, ARROW_COL);
    footnote(f, "moving past you (gamma = 2)", 16, LABEL_COL);
}

// ============================================================================
// Panel 6: relativistic Doppler shift
// ============================================================================

const float P6_BETA = 0.9f;

void p6A(const Fig& f) {
    BoostFrame b = boostOf({ P6_BETA, 0.0f });
    for (int i = 0; i < 8; ++i) {
        float phi = 6.2831853f * i / 8.0f;
        Vec2  dir = { std::cos(phi), std::sin(phi) };
        Vec2  p   = { f.c.x + 125.0f * dir.x, f.c.y + 85.0f * dir.y };
        float D   = relativity::dopplerFactor(b.n.dot(dir), b);
        Color col = hueShiftFromDoppler(DOPPLER_BASE, D);
        glowDot(f, p, 30.0f, withAlpha(col, 0.9f));
        drone(f, p, lerpColor(col, WHITE, 0.25f));
    }
    ship(f, f.c);
    arrow({ f.c.x + 20.0f, f.c.y + 4.0f }, { f.c.x + 70.0f, f.c.y + 4.0f }, ARROW_COL);
    text("0.9c", f.c.x + 24.0f, f.c.y + 10.0f, 14, ARROW_COL);
}

void p6B(const Fig& f) {
    BoostFrame b = boostOf({ P6_BETA, 0.0f });
    const float dMin = 0.2f, dMax = 5.0f;
    float x0 = f.box.x + 30.0f, w = f.box.width - 60.0f;
    float y  = f.c.y - 30.0f, h = 28.0f;
    auto xOf = [&](float D) {
        return x0 + w * std::log(D / dMin) / std::log(dMax / dMin);
    };
    for (int i = 0; i < (int)w; ++i) {
        float D = dMin * std::pow(dMax / dMin, i / w);
        DrawRectangle((int)(x0 + i), (int)y, 1, (int)h, hueShiftFromDoppler(DOPPLER_BASE, D));
    }
    DrawRectangleLinesEx({ x0 - 1, y - 1, w + 2, h + 2 }, 1.0f, PANEL_EDGE);
    text("< redder", x0, y - 22.0f, 16, LABEL_COL);
    text("bluer >", x0 + w - MeasureText("bluer >", 16), y - 22.0f, 16, LABEL_COL);

    struct Mark { const char* name; float D; int row; };
    const Mark marks[4] = {
        { "behind", relativity::dopplerFactor(-1.0f, b), 0 },
        { "side",   relativity::dopplerFactor( 0.0f, b), 1 },
        { "at rest", 1.0f,                               0 },
        { "ahead",  relativity::dopplerFactor( 1.0f, b), 1 },
    };
    for (const Mark& m : marks) {
        float x  = xOf(m.D);
        float ty = y + h + 6.0f + m.row * 38.0f;
        DrawLineEx({ x, y + h }, { x, ty }, 1.0f, LABEL_COL);
        textCentred(m.name, x, ty + 2.0f, 16, LABEL_COL);
        textCentred(TextFormat("D=%.2f", m.D), x, ty + 19.0f, 14, GUIDE_COL);
    }
    footnote(f, "D = observed / emitted frequency  (v = 0.9c)", 14, LABEL_COL);
}

// ============================================================================
// Panel 7: stellar aberration
// ============================================================================

void p7Stars(const Fig& f, const BoostFrame* b) {
    unsigned seed = 2024u;
    Vec2 s = f.c;
    for (int i = 0; i < 520; ++i) {
        Vec2  p  = { s.x + (rnd(seed) - 0.5f) * 2.0f * BOX_W,
                     s.y + (rnd(seed) - 0.5f) * 2.0f * BOX_H };
        float br = 0.4f + 0.6f * rnd(seed);
        Color col = STAR_COL;
        if (b) {
            Vec2  d    = p - s;
            float dist = d.length();
            // aberrate() acts on photon directions: transform the light
            // travelling star → ship, then flip to get the apparent direction.
            Vec2  ab   = -relativity::aberrate(-(d / dist), *b, /*toShip=*/true);
            p   = s + ab * dist;
            col = hueShiftFromDoppler(STAR_COL, relativity::dopplerFactor(b->n.dot(ab), *b));
        }
        DrawCircleV(rv(p), 1.6f, withAlpha(col, br));
    }
    ship(f, s);
}

void p7A(const Fig& f) { p7Stars(f, nullptr); }

void p7B(const Fig& f) {
    BoostFrame b = boostOf({ 0.0f, -0.9f });
    p7Stars(f, &b);
    arrow({ f.c.x + 24.0f, f.c.y + 10.0f }, { f.c.x + 24.0f, f.c.y - 40.0f }, ARROW_COL);
    text("0.9c", f.c.x + 30.0f, f.c.y - 34.0f, 16, ARROW_COL);
}

const Panel PANELS[] = {
    { "Relative motion",
      "Your frame", p1A, "The drone's frame", p1B,
      "In ordinary life, we are used to velocities that depend on your frame of "
      "reference: sitting in a moving plane, we feel that we are at rest, but to an "
      "observer on the ground we go whizzing by." },

    { "A constant speed of light",
      "Ordinary bullets (drone's frame)", p2A, "Light-speed bolts (drone's frame)", p2B,
      "In special relativity, the speed of light is constant in all reference frames. "
      "In ordinary life, this is so fast that we don't notice. But in Relativistic "
      "Galaga, your bolts move at lightspeed, letting you see how "
      "the universe would look if it had a slower speed of light." },

    { "Your frame of reference",
      "Lab frame: you are moving", p3A, "Your frame: the game window", p3B,
      "The game window shows your Lorentz frame: the way that distances and times would "
      "appear to an observer making measurements in the rest frame of your ship." },

    { "Time dilation",
      "A light clock, on board", p4A, "The same clock, seen from the lab", p4B,
      "Moving clocks run slow. Imagine a clock that ticks each time a pulse of light "
      "bounces between two mirrors. On your ship the light goes straight up and down. "
      "Seen by an observer at rest among the stars, the ship moves while the light is in "
      "flight, so the light follows a longer, slanted path, and since light always "
      "travels at c, each tick takes longer. The slow-down factor is the Lorentz factor, "
      "gamma~=~1~/~sqrt(1~-~v^2/c^2). In the game, compare the lab clock t_lab with your "
      "ship's clock tau_ship." },

    { "Length contraction",
      "At rest", p5A, "Moving at 0.87c", p5B,
      "An object moving relative to you is measured to be shorter along its direction "
      "of motion, by the same factor gamma; its size across the motion is unchanged. "
      "When your ship moves, everything at rest among the stars (enemies, the gaps "
      "between them, the stars themselves) is moving relative to you, so in the game "
      "window it is squashed along your direction of travel." },

    { "Relativistic Doppler shift",
      "Moving right at 0.9c", p6A, "Colour shift vs. direction", p6B,
      "Light from something you are moving towards arrives with its waves bunched up: "
      "higher frequency, shifted towards blue. Light from behind you is stretched out "
      "and shifted towards red. Relativity adds a twist: because of time dilation, light "
      "arriving from directly to the side is redshifted too. Real shifts would quickly "
      "push colours out of the visible range, so the game rotates them around the "
      "colour wheel instead." },

    { "Stellar aberration",
      "At rest", p7A, "Moving up at 0.9c", p7B,
      "Motion changes the direction light appears to come from. Run through falling "
      "rain and it seems to come at you from the front; in the same way, as your ship "
      "speeds up, light from the stars is tilted towards your direction of travel, so "
      "stars crowd into the sky ahead of you and thin out behind. The same effect tilts "
      "your bolts: fired straight ahead in your frame, they travel at an angle in the "
      "lab frame." },
};


} // namespace

const Deck& scienceDeck() {
    static const Deck deck{ "THE SCIENCE", PANELS, (int)(sizeof(PANELS) / sizeof(PANELS[0])) };
    return deck;
}
