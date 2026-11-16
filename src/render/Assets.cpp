#include "Assets.h"
#include <raylib.h>
#include <algorithm>
#include <cmath>
#include <string>

#ifndef ASSETS_DIR
#define ASSETS_DIR "assets/"
#endif

static Texture2D tryLoad(const char* filename) {
    std::string path = std::string(ASSETS_DIR) + filename;
    if (FileExists(path.c_str()))
        return LoadTexture(path.c_str());
    return {};
}

// Load a sprite converted to (brightened) greyscale, keeping alpha, so that a
// draw-time tint fully determines its colour (used for Doppler-tinted effects).
static Texture2D tryLoadGrey(const char* filename) {
    std::string path = std::string(ASSETS_DIR) + filename;
    if (!FileExists(path.c_str())) return {};
    Image img = LoadImage(path.c_str());
    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    auto* px = static_cast<unsigned char*>(img.data);
    for (int i = 0; i < img.width * img.height; ++i) {
        unsigned char* p = px + 4 * i;
        float l = 0.30f * p[0] + 0.59f * p[1] + 0.11f * p[2];
        auto  g = static_cast<unsigned char>(std::min(255.0f, l * 1.3f));
        p[0] = p[1] = p[2] = g;
    }
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

// White texture whose alpha is a radial profile f(r), r in [0,1] from the centre.
template <typename F>
static Texture2D makeRadial(int size, F profile) {
    Image img = GenImageColor(size, size, BLANK);
    auto* px = static_cast<unsigned char*>(img.data);   // R8G8B8A8
    float h = 0.5f * (size - 1);
    for (int y = 0; y < size; ++y)
        for (int x = 0; x < size; ++x) {
            float r = std::sqrt((x - h) * (x - h) + (y - h) * (y - h)) / h;
            float a = (r >= 1.0f) ? 0.0f : std::clamp(profile(r), 0.0f, 1.0f);
            unsigned char* p = px + 4 * (y * size + x);
            p[0] = p[1] = p[2] = 255;
            p[3] = static_cast<unsigned char>(255.0f * a);
        }
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

void Assets::load() {
    ship         = tryLoad("ship.png");
    drone        = tryLoad("drone.png");
    fighter      = tryLoad("enemy_fighter.png");
    heavy        = tryLoad("enemy_heavy.png");
    enemy        = tryLoad("enemy.png");
    boltPlayer   = tryLoadGrey("bolt_player.png");
    boltEnemy    = tryLoadGrey("bolt_enemy.png");
    boltHeavy    = tryLoadGrey("bolt_heavy.png");
    fxFlash      = tryLoadGrey("fx_explosion_flash.png");
    glow = makeRadial(64,  [](float r) { float s = 1.0f - r; return s * s; });
    ring = makeRadial(128, [](float r) { float d = (r - 0.82f) / 0.09f;
                                         return std::exp(-d * d); });
    powerupBoost = tryLoad("powerup_boost.png");
    powerupHeal  = tryLoad("powerup_heal.png");
    powerupTwin  = tryLoad("powerup_twin.png");
    fxHitEnemy   = tryLoadGrey("fx_hit_enemy.png");
    fxHitPlayer  = tryLoadGrey("fx_hit_player.png");

    for (Texture2D* t : { &fighter, &heavy, &powerupBoost, &powerupHeal,
                          &powerupTwin, &fxHitEnemy, &fxHitPlayer,
                          &boltPlayer, &boltEnemy, &boltHeavy, &fxFlash,
                          &glow, &ring })
        if (t->id > 0) SetTextureFilter(*t, TEXTURE_FILTER_BILINEAR);
}

void Assets::unload() {
    auto u = [](Texture2D& t) { if (t.id > 0) { UnloadTexture(t); t = {}; } };
    u(ship);  u(drone);  u(fighter);  u(heavy);  u(enemy);
    u(boltPlayer);  u(boltEnemy);  u(boltHeavy);  u(fxFlash);
    u(glow);  u(ring);
    u(powerupBoost);  u(powerupHeal);  u(powerupTwin);
    u(fxHitEnemy);  u(fxHitPlayer);
}
