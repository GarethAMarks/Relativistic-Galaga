#include "Starfield.h"
#include "ColorRemap.h"
#include "relativity/Aberration.h"
#include "relativity/Doppler.h"
#include <raylib.h>
#include <cstdlib>

Starfield::Starfield() {
    srand(42);
    for (auto& s : stars_) {
        // Spread over a 1600x1200 field centred on the screen (400, 300).
        // Extends one screen-width offscreen in each direction so the contracted
        // field still fills the screen at gamma ~2 (beta ~0.87).
        s.x          = -400.0f + static_cast<float>(rand() % 1600);
        s.y          = -300.0f + static_cast<float>(rand() % 1200);
        s.brightness = 0.4f + 0.6f * (static_cast<float>(rand()) / RAND_MAX);
    }
}

void Starfield::update(Vec2 shipVel, float dt, bool lorentzianMode) {
    (void)shipVel; (void)dt; (void)lorentzianMode;  // fixed background — no scrolling
}

void Starfield::draw(const BoostFrame& boost, bool lorentzianMode, Vec2 shipPos) const {
    for (const auto& s : stars_) {
        float drawX = s.x, drawY = s.y;
        unsigned char alpha = static_cast<unsigned char>(s.brightness * 200.0f);
        Color col = {255, 255, 255, alpha};

        if (lorentzianMode && boost.beta > 0.001f) {
            Vec2 delta = Vec2{s.x, s.y} - shipPos;
            float dist = delta.length();
            if (dist > 1.0f) {
                // Aberrate the direction: stars bunch toward the forward hemisphere.
                // aberrate() transforms PHOTON (propagation) directions, so
                // transform the light travelling from the star towards the
                // ship (-dir), then flip back to get the apparent star direction.
                Vec2 dir    = delta / dist;
                Vec2 abDir  = -relativity::aberrate(-dir, boost, /*toShip=*/true);
                drawX = shipPos.x + abDir.x * dist;
                drawY = shipPos.y + abDir.y * dist;

                // Doppler uses the ship-frame (aberrated) angle, not the lab-frame angle.
                float cosTheta = boost.n.dot(abDir);
                float D        = relativity::dopplerFactor(cosTheta, boost);
                Color shifted  = render::hueShiftFromDoppler(Color{220, 230, 255, 255}, D);
                col = Color{shifted.r, shifted.g, shifted.b, alpha};
            }
        }

        if (drawX >= 0.0f && drawX < 800.0f && drawY >= 0.0f && drawY < 600.0f)
            DrawPixel(static_cast<int>(drawX), static_cast<int>(drawY), col);
    }
}
