#include "Renderer.h"
#include "Assets.h"
#include "sim/Powerup.h"
#include "sim/Explosion.h"
#include "sim/MuzzleFlash.h"
#include "sim/HitEffect.h"
#include "ColorRemap.h"
#include "DrawUtil.h"
#include "relativity/Lorentz.h"
#include "relativity/Doppler.h"
#include <raylib.h>
#include <rlgl.h>
#include <algorithm>
#include <cmath>

using namespace render;

static float easeOut(float t) { t = std::clamp(t, 0.0f, 1.0f); return 1.0f - (1.0f - t) * (1.0f - t); }

// ---- Per-type sprite/colour tables -----------------------------------------

static const Texture2D& enemyTexture(EnemyKind k, const Assets& a) {
    switch (k) {
        case EnemyKind::Drone:   return a.drone;
        case EnemyKind::Fighter: return a.fighter;
        case EnemyKind::Heavy:   return a.heavy;
        default:                 return a.enemy;
    }
}

// Base tint for each enemy type.  This multiplies the sprite and is the colour
// that gets Doppler hue-shifted in Lorentzian mode.
static Color enemyBaseColor(EnemyKind k) {
    switch (k) {
        case EnemyKind::Fighter: return Color{255, 180, 140, 255};
        case EnemyKind::Heavy:   return Color{192, 140, 255, 255};
        default:                 return SKYBLUE;
    }
}

static const Texture2D& powerupTexture(PowerupKind k, const Assets& a) {
    switch (k) {
        case PowerupKind::Boost: return a.powerupBoost;
        case PowerupKind::Heal:  return a.powerupHeal;
        default:                 return a.powerupTwin;
    }
}

// Half-extents for a powerup sprite: longest side = 2*RADIUS, aspect preserved.
static void powerupExtents(const Texture2D& tex, float& hw, float& hh) {
    hw = hh = Powerup::RADIUS;
    if (tex.id > 0) {
        float m = (float)std::max(tex.width, tex.height);
        hw = Powerup::RADIUS * tex.width  / m;
        hh = Powerup::RADIUS * tex.height / m;
    }
}

static const Color ENEMY_HIT_FX_COLOR  = {255, 235, 140, 255};  // player-bolt yellow
static const Color PLAYER_HIT_FX_COLOR = {255,  90,  50, 255};

// ---- Bolts -----------------------------------------------------------------
// Base (un-shifted) glow colours; Lorentzian mode Doppler-shifts these.
static const Color PLAYER_BOLT_COLOR = YELLOW;
static const Color ENEMY_BOLT_COLOR  = ORANGE;
static const Color HEAVY_BOLT_COLOR  = Color{255, 50, 40, 255};

// Draws a glowing laser bolt: a tinted (greyscale) laser sprite for the
// coloured glow, plus a narrower near-white core.  Drawn additively by the
// caller.  The sprite is longer than the hitbox; it is shifted back so its
// bright head sits at the front of the hitbox and the tail trails behind.
static void drawBolt(const Bolt& b, Vec2 centre, Color glowCol, bool isPlayer,
                     const Assets& a, const BoostFrame* boost) {
    const Texture2D& tex = isPlayer ? a.boltPlayer : (b.big ? a.boltHeavy : a.boltEnemy);
    float hw = isPlayer ? 3.5f  : (b.big ? 7.0f  : 3.5f);
    float hh = isPlayer ? 17.0f : (b.big ? 16.0f : 11.0f);

    float speed = b.vel.length();
    Vec2  dir   = speed > 1e-3f ? b.vel / speed : Vec2{0.0f, isPlayer ? -1.0f : 1.0f};
    float ang   = headingAngle(dir);
    Vec2  c     = centre - shapeOffset(dir * std::max(0.0f, hh - b.hh), boost);

    drawSpriteQuad(tex, c, hw, hh, ang, boost, glowCol);
    drawSpriteQuad(tex, c, hw * 0.45f, hh * 0.92f, ang, boost,
                   lerpColor(glowCol, WHITE, 0.6f));
}

// ---- Explosions ------------------------------------------------------------

// Cheap deterministic hash → [0,1), so each explosion's sparks are stable.
static float hash01(unsigned seed, unsigned i) {
    unsigned h = seed * 747796405u + i * 2891336453u + 1u;
    h ^= h >> 16; h *= 0x7feb352du; h ^= h >> 15; h *= 0x846ca68bu; h ^= h >> 16;
    return (h & 0xFFFFFF) / 16777216.0f;
}

// Layered explosion: star-burst flash, fireball glow, shockwave ring, sparks.
// All layers go through drawSpriteQuad, so in Lorentzian mode the whole thing
// is contracted exactly like a sprite.  `D` Doppler-shifts every colour.
// Drawn additively by the caller.
static void drawExplosionAt(const Explosion& ex, Vec2 screenPos,
                            const BoostFrame* boost, float D, const Assets& a) {
    float T = std::clamp(ex.age / Explosion::DURATION, 0.0f, 1.0f);
    float R = Explosion::MAX_RADIUS * ex.scale;
    auto  shift = [D](Color c) { return render::hueShiftFromDoppler(c, D); };

    // Fireball: hot yellow → orange → deep red as it cools and fades.
    {
        Color hot  = Color{255, 220, 120, 255};
        Color warm = Color{255, 110,  30, 255};
        Color cool = Color{170,  30,  20, 255};
        Color col  = (T < 0.4f) ? lerpColor(hot, warm, T / 0.4f)
                                : lerpColor(warm, cool, (T - 0.4f) / 0.6f);
        float size = R * (0.35f + 0.55f * easeOut(T * 1.4f));
        float alpha = std::pow(1.0f - T, 1.8f);
        drawSpriteQuad(a.glow, screenPos, size, size, 0.0f, boost,
                       withAlpha(shift(col), alpha));
        // White-hot centre early on.
        if (T < 0.45f)
            drawSpriteQuad(a.glow, screenPos, size * 0.5f, size * 0.5f, 0.0f, boost,
                           withAlpha(shift(Color{255, 250, 220, 255}), 1.0f - T / 0.45f));
    }

    // Shockwave ring.
    {
        float size  = R * (0.3f + 1.1f * easeOut(T));
        float alpha = 0.85f * std::pow(1.0f - T, 3.0f);
        drawSpriteQuad(a.ring, screenPos, size, size, 0.0f, boost,
                       withAlpha(shift(Color{255, 170, 90, 255}), alpha));
    }

    // Star-burst flash in the first quarter.
    if (T < 0.25f) {
        float tf   = T / 0.25f;
        float size = R * 0.8f * (0.5f + 0.5f * tf);
        float rot  = hash01(ex.seed, 999) * 6.2831853f;
        drawSpriteQuad(a.fxFlash, screenPos, size, size, rot, boost,
                       withAlpha(shift(Color{255, 245, 200, 255}), 1.0f - tf));
    }

    // Sparks: short streaks flying outward and decelerating.
    int n = 8 + (int)(4.0f * ex.scale);
    for (int i = 0; i < n; ++i) {
        float th    = hash01(ex.seed, 2 * i) * 6.2831853f;
        float reach = R * (1.0f + 1.2f * hash01(ex.seed, 2 * i + 1));
        float life  = 0.55f + 0.45f * hash01(ex.seed, 100 + i);  // fraction of DURATION
        float ts    = T / life;
        if (ts >= 1.0f) continue;
        Vec2  dir   = { std::cos(th), std::sin(th) };
        Vec2  p     = screenPos + shapeOffset(dir * (reach * easeOut(ts)), boost);
        float len   = 6.0f * (1.0f - ts) + 1.5f;
        drawSpriteQuad(a.boltEnemy, p, 2.0f, len, headingAngle(dir), boost,
                       withAlpha(shift(Color{255, 200, 110, 255}), 1.0f - ts));
    }
}

// ---- Muzzle flash ----------------------------------------------------------

// Soft glow puff at the gun, contracted in Lorentzian mode.  Drawn additively.
static void drawMuzzleFlashAt(const MuzzleFlash& f, Vec2 screenPos,
                              const Assets& a, const BoostFrame* boost) {
    float t = std::clamp(f.age / MuzzleFlash::DURATION, 0.0f, 1.0f);
    float s = 1.0f + 0.6f * t;
    drawSpriteQuad(a.glow, screenPos, MuzzleFlash::RADIUS_H * 2.0f * s,
                   MuzzleFlash::RADIUS_V * 1.6f * s, 0.0f, boost,
                   withAlpha(f.color, 1.0f - t));
    drawSpriteQuad(a.glow, screenPos, MuzzleFlash::RADIUS_H * 0.8f,
                   MuzzleFlash::RADIUS_V * 0.7f, 0.0f, boost,
                   withAlpha(WHITE, 0.8f * (1.0f - t)));
}

// ---- Hit effect draw helper ------------------------------------------------

// A burst sprite that pops outward and fades, plus a bright core.
static void drawHitEffectAt(const HitEffect& h, Vec2 screenPos, Color base,
                            const Texture2D& tex, const Assets& a,
                            const BoostFrame* boost) {
    float t     = std::clamp(h.age / HitEffect::DURATION, 0.0f, 1.0f);
    float size  = (h.onPlayer ? HitEffect::SIZE_PLAYER : HitEffect::SIZE_ENEMY)
                * (0.45f + 0.75f * easeOut(t));
    float rot   = (h.rotation + 90.0f * t) * DEG2RAD;

    drawSpriteQuad(tex, screenPos, size, size, rot, boost, withAlpha(base, 1.0f - t * t));
    if (t < 0.5f)   // white-hot core
        drawSpriteQuad(a.glow, screenPos, size * 0.6f, size * 0.6f, 0.0f, boost,
                       withAlpha(WHITE, 0.9f * (1.0f - 2.0f * t)));
}

// ---- Newtonian draw --------------------------------------------------------

static void drawNewtonian(const Simulation& sim, const Assets& assets) {
    for (const auto& e : sim.enemies()) {
        if (!e.alive) continue;
        drawSpriteQuad(assets.enemy, e.pos, Enemy::HW, Enemy::HH, 0.0f, nullptr, SKYBLUE);
    }
    for (const auto& ep : sim.hostiles()) {
        const Enemy& e = *ep;
        if (!e.alive) continue;
        Color col = enemyBaseColor(e.kind());
        if (e.hitFlash > 0.0f) col = lerpColor(col, WHITE, 0.8f);
        drawSpriteQuad(enemyTexture(e.kind(), assets), e.pos,
                       e.halfW(), e.halfH(), 0.0f, nullptr, col);
    }
    for (const auto& p : sim.powerups()) {
        if (!p->alive) continue;
        const Texture2D& tex = powerupTexture(p->kind(), assets);
        float hw, hh;
        powerupExtents(tex, hw, hh);
        drawSpriteQuad(tex, p->pos, hw, hh, 0.0f, nullptr, p->color());
    }

    BeginBlendMode(BLEND_ADDITIVE);
    for (const auto& b : sim.bolts()) {
        if (!b.alive) continue;
        drawBolt(b, b.pos, PLAYER_BOLT_COLOR, true, assets, nullptr);
    }
    for (const auto& b : sim.enemyBolts()) {
        if (!b.alive) continue;
        drawBolt(b, b.pos, b.big ? HEAVY_BOLT_COLOR : ENEMY_BOLT_COLOR, false,
                 assets, nullptr);
    }
    for (const auto& ex : sim.explosions()) {
        if (!ex.alive) continue;
        drawExplosionAt(ex, ex.pos, nullptr, 1.0f, assets);
    }
    for (const auto& f : sim.muzzleFlashes()) {
        if (!f.alive) continue;
        drawMuzzleFlashAt(f, f.pos, assets, nullptr);
    }
    for (const auto& h : sim.hitEffects()) {
        if (!h.alive) continue;
        drawHitEffectAt(h, h.pos,
                        h.onPlayer ? PLAYER_HIT_FX_COLOR : ENEMY_HIT_FX_COLOR,
                        h.onPlayer ? assets.fxHitPlayer : assets.fxHitEnemy,
                        assets, nullptr);
    }
    EndBlendMode();
}

// ---- Lorentzian draw -------------------------------------------------------

// Ship-frame position of a uniformly-moving object (closed-form Eq. 9).
static Vec2 transformedPos(Vec2 r0, Vec2 vel, float t0,
                            float tPrime, const BoostFrame& boost) {
    float ti = relativity::solveSimultaneousLabTime(r0, vel, t0, tPrime, boost);
    Vec2  ri = r0 + vel * (ti - t0);
    return relativity::toShipFrame({ri, ti}, boost).r;
}

// Ship-frame position of an enemy on an arbitrary lab trajectory r(t).
// Start from the uniform-motion solution, then refine with Newton iterations on
//   f(t) = gamma * (t - v.r(t)/c^2) - t' = 0,   f'(t) = gamma * (1 - v.u(t)/c^2).
// For uniform motion the first guess is exact and the loop is a no-op.
static Vec2 transformedEnemyPos(const Enemy& e, float t_lab, float tPrime,
                                 const BoostFrame& boost) {
    float c2 = boost.c * boost.c;
    float ti = relativity::solveSimultaneousLabTime(e.pos, e.vel, t_lab, tPrime, boost);
    for (int k = 0; k < 3; ++k) {
        Vec2  r  = e.posAt(ti);
        Vec2  u  = e.velAt(ti);
        float f  = boost.gamma * (ti - boost.v.dot(r) / c2) - tPrime;
        float fp = boost.gamma * (1.0f - boost.v.dot(u) / c2);
        if (std::fabs(fp) < 1e-6f) break;
        float step = f / fp;
        ti -= step;
        if (std::fabs(step) < 1e-6f) break;
    }
    return relativity::toShipFrame({e.posAt(ti), ti}, boost).r;
}

static void drawLorentzian(const Simulation& sim, const BoostFrame& boost,
                            const Assets& assets) {
    float t_lab   = sim.labTime();
    Vec2  shipPos = sim.ship().pos;

    auto  shipEvent    = relativity::toShipFrame({shipPos, t_lab}, boost);
    float t_prime      = shipEvent.t;
    Vec2  r_ship_prime = shipEvent.r;

    Vec2 anchor = shipPos - r_ship_prime;

    // Doppler factor for a lab-frame source seen at ship-frame position r_prime.
    auto dopplerAt = [&](Vec2 r_prime) {
        Vec2  to  = r_prime - r_ship_prime;
        float len = to.length();
        float cosTheta = len > 1.0f ? boost.n.dot(to / len) : 0.0f;
        return relativity::dopplerFactor(cosTheta, boost);
    };

    // ----- Formation enemies -----
    for (const auto& e : sim.enemies()) {
        if (!e.alive) continue;
        Vec2 r_prime = transformedPos(e.pos, e.vel, t_lab, t_prime, boost);
        Color col    = render::hueShiftFromDoppler(SKYBLUE, dopplerAt(r_prime));
        drawSpriteQuad(assets.enemy, anchor + r_prime, Enemy::HW, Enemy::HH,
                       0.0f, &boost, col);
    }

    // ----- Drones, fighters, heavies -----
    for (const auto& ep : sim.hostiles()) {
        const Enemy& e = *ep;
        if (!e.alive) continue;
        Vec2  r_prime = transformedEnemyPos(e, t_lab, t_prime, boost);
        Color col     = render::hueShiftFromDoppler(enemyBaseColor(e.kind()),
                                                    dopplerAt(r_prime));
        if (e.hitFlash > 0.0f) col = lerpColor(col, WHITE, 0.8f);
        drawSpriteQuad(enemyTexture(e.kind(), assets), anchor + r_prime,
                       e.halfW(), e.halfH(), 0.0f, &boost, col);
    }

    // ----- Powerups (uniform lab motion: transformed, contracted, Doppler-tinted) -----
    for (const auto& p : sim.powerups()) {
        if (!p->alive) continue;
        Vec2  r_prime = transformedPos(p->pos, p->vel, t_lab, t_prime, boost);
        Color col     = render::hueShiftFromDoppler(p->color(), dopplerAt(r_prime));
        const Texture2D& tex = powerupTexture(p->kind(), assets);
        float hw, hh;
        powerupExtents(tex, hw, hh);
        drawSpriteQuad(tex, anchor + r_prime, hw, hh, 0.0f, &boost, col);
    }

    BeginBlendMode(BLEND_ADDITIVE);

    // ----- Player bolts (drawn at lab-frame pos so visual speed = c_game;
    //       colour is the spawn-time cached Doppler tint) -----
    for (const auto& b : sim.bolts()) {
        if (!b.alive) continue;
        Color col = render::hueShiftFromDoppler(PLAYER_BOLT_COLOR, b.doppler);
        drawBolt(b, b.pos, col, true, assets, &boost);
    }

    // ----- Enemy bolts (lab-frame, Doppler-shifted by ship velocity) -----
    for (const auto& b : sim.enemyBolts()) {
        if (!b.alive) continue;
        Vec2  tobolt    = b.pos - shipPos;
        float len       = tobolt.length();
        float cosTheta  = len > 1.0f ? boost.n.dot(tobolt / len) : 0.0f;
        float D         = relativity::dopplerFactor(cosTheta, boost);
        Color col       = render::hueShiftFromDoppler(
                              b.big ? HEAVY_BOLT_COLOR : ENEMY_BOLT_COLOR, D);
        drawBolt(b, b.pos, col, false, assets, &boost);
    }

    // ----- Explosions (stationary in lab frame: contracted + Doppler-shifted) -----
    for (const auto& ex : sim.explosions()) {
        if (!ex.alive) continue;
        Vec2 r_prime = transformedPos(ex.pos, {0.0f, 0.0f}, 0.0f, t_prime, boost);
        drawExplosionAt(ex, anchor + r_prime, &boost, dopplerAt(r_prime), assets);
    }

    // ----- Muzzle flashes (stationary in lab frame, Lorentz-contracted) -----
    for (const auto& f : sim.muzzleFlashes()) {
        if (!f.alive) continue;
        Vec2 r_prime = transformedPos(f.pos, {0.0f, 0.0f}, 0.0f, t_prime, boost);
        drawMuzzleFlashAt(f, anchor + r_prime, assets, &boost);
    }

    // ----- Hit effects -----
    // On enemies: ride with the enemy that was hit (transformed, contracted,
    // Doppler-tinted like the enemy).  On the player: ride with the ship, which
    // is the observer, so drawn at rest and unshifted.
    for (const auto& h : sim.hitEffects()) {
        if (!h.alive) continue;
        if (h.onPlayer) {
            drawHitEffectAt(h, h.pos, PLAYER_HIT_FX_COLOR, assets.fxHitPlayer,
                            assets, nullptr);
        } else {
            Vec2  r_prime = transformedPos(h.pos, h.vel, t_lab, t_prime, boost);
            Color col     = render::hueShiftFromDoppler(ENEMY_HIT_FX_COLOR,
                                                        dopplerAt(r_prime));
            drawHitEffectAt(h, anchor + r_prime, col, assets.fxHitEnemy,
                            assets, &boost);
        }
    }

    EndBlendMode();
}

// ---- Public entry point ----------------------------------------------------

void Renderer::draw(const Simulation& sim,
                    const BoostFrame& boost,
                    bool lorentzianMode,
                    float c_game,
                    const Assets& assets) const {
    (void)c_game;

    if (!lorentzianMode) {
        drawNewtonian(sim, assets);
    } else {
        drawLorentzian(sim, boost, assets);
    }

    // Ship sprite: 26 px wide × 52 px tall, nose at top.  Blinks red after a hit.
    Color shipTint = WHITE;
    float hitT = sim.playerHitTimer();
    if (hitT > 0.0f && (int)(hitT * 20.0f) % 2 == 0)
        shipTint = Color{255, 80, 80, 255};

    const Vec2& sp = sim.ship().pos;
    if (sim.isGameOver()) {
        // Ship destroyed — only the explosion remains.
    } else if (assets.ship.id > 0) {
        Rectangle src = { 0, 0, (float)assets.ship.width, (float)assets.ship.height };
        Rectangle dst = { sp.x - 13.0f, sp.y - 35.0f, 26.0f, 52.0f };
        DrawTexturePro(assets.ship, src, dst, { 0, 0 }, 0.0f, shipTint);
    } else {
        Vector2 apex   = { sp.x,          sp.y - 35.0f };
        Vector2 bLeft  = { sp.x - 13.0f,  sp.y + 17.0f };
        Vector2 bRight = { sp.x + 13.0f,  sp.y + 17.0f };
        DrawTriangle(apex, bLeft, bRight, shipTint);
        DrawRectangle((int)(sp.x - 4), (int)(sp.y + 17), 8, 4, ORANGE);
    }

    // Brief red vignette at the screen edges when the hull is hit.
    if (hitT > 0.0f) {
        float a = 0.45f * hitT / Simulation::PLAYER_HIT_FLASH;
        Color edge = ColorAlpha(RED, a);
        Color none = ColorAlpha(RED, 0.0f);
        const int w = 800, h = 600, d = 36;
        DrawRectangleGradientV(0, 0,     w, d, edge, none);
        DrawRectangleGradientV(0, h - d, w, d, none, edge);
        DrawRectangleGradientH(0, 0,     d, h, edge, none);
        DrawRectangleGradientH(w - d, 0, d, h, none, edge);
    }
}
