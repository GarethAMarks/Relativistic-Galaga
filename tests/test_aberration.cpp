#include "relativity/Aberration.h"
#include <cmath>
#include <cstdio>

static constexpr float TOL = 1e-4f;
static constexpr float C   = 300.0f;

#define CHECK(expr) \
    do { if (!(expr)) { \
        std::printf("  FAIL at %s:%d: " #expr "\n", __FILE__, __LINE__); \
        return false; \
    } } while(0)

#define CHECK_NEAR(a, b) \
    CHECK(std::fabs((a) - (b)) < TOL)

bool testAberration() {
    using namespace relativity;

    // --- Identity at beta=0 ---
    {
        BoostFrame f = BoostFrame::fromVelocity({0.0f, 0.0f}, C);
        Vec2 dir = Vec2{1.0f, -1.0f}.normalized();
        Vec2 out = aberrate(dir, f, true);
        CHECK_NEAR(out.x, dir.x);
        CHECK_NEAR(out.y, dir.y);
    }

    // --- Round-trip lab -> ship -> lab ---
    {
        BoostFrame f = BoostFrame::fromVelocity({80.0f, -200.0f}, C);
        Vec2 dir = Vec2{1.0f, -2.0f}.normalized();
        Vec2 ship = aberrate(dir, f, true);
        Vec2 back = aberrate(ship, f, false);
        CHECK_NEAR(back.x, dir.x);
        CHECK_NEAR(back.y, dir.y);
    }

    // --- Speed invariance: aberrated light ray still has |vel| == c ---
    // If dir is a lab-frame light-ray direction, the ship-frame direction
    // must also be a unit vector (so multiplied by c it gives speed c).
    {
        BoostFrame f = BoostFrame::fromVelocity({-100.0f, -220.0f}, C);
        // Several lab-frame ray directions
        const Vec2 dirs[] = {
            { 0.0f, -1.0f },
            Vec2{1.0f, -1.0f}.normalized(),
            Vec2{-0.5f, -0.866f}.normalized(),
            { 1.0f, 0.0f },
        };
        for (const auto& d : dirs) {
            Vec2 shipDir = aberrate(d, f, true);
            float speed = shipDir.length();
            CHECK_NEAR(speed, 1.0f);  // unit vector => speed = c when scaled
        }
    }

    // --- Forward direction stays forward ---
    {
        BoostFrame f = BoostFrame::fromVelocity({0.0f, -180.0f}, C);
        // n = {0, -1}; input along n stays along n after aberration.
        Vec2 forward = {0.0f, -1.0f};
        Vec2 out = aberrate(forward, f, true);
        CHECK_NEAR(out.x, 0.0f);
        CHECK_NEAR(out.y, -1.0f);
    }

    // --- Headlight effect: a star perpendicular to the ship's motion appears
    // in the forward hemisphere in the ship frame.
    // aberrate() operates on PHOTON directions (direction of travel), not
    // source directions.  A star to the right sends photons traveling leftward.
    {
        BoostFrame f = BoostFrame::fromVelocity({0.0f, -240.0f}, C);  // beta=0.8, n={0,-1}
        // Photon arriving from a star to the right travels in the -x direction:
        Vec2 photonLab = {-1.0f, 0.0f};
        Vec2 photonShip = aberrate(photonLab, f, true);
        // Apparent star direction = -photonShip.  For forward bunching it must
        // have a negative-y component (toward n={0,-1}).  That means photonShip.y > 0.
        CHECK(photonShip.y > 0.0f);
    }

    return true;
}
