#include "relativity/Lorentz.h"
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

bool testLorentz() {
    using namespace relativity;

    // --- Identity at beta=0 ---
    {
        BoostFrame f = BoostFrame::fromVelocity({0.0f, 0.0f}, C);
        Event lab = { {120.0f, -80.0f}, 3.5f };
        Event ship = toShipFrame(lab, f);
        CHECK_NEAR(ship.r.x, lab.r.x);
        CHECK_NEAR(ship.r.y, lab.r.y);
        CHECK_NEAR(ship.t,   lab.t);
    }

    // --- Round-trip lab -> ship -> lab ---
    {
        BoostFrame f = BoostFrame::fromVelocity({60.0f, -200.0f}, C);
        Event lab = { {250.0f, -150.0f}, 2.0f };
        Event ship = toShipFrame(lab, f);
        Event back = toLabFrame(ship, f);
        CHECK_NEAR(back.r.x, lab.r.x);
        CHECK_NEAR(back.r.y, lab.r.y);
        CHECK_NEAR(back.t,   lab.t);
    }

    // --- Simultaneity solve: result should be consistent with toShipFrame ---
    {
        BoostFrame f = BoostFrame::fromVelocity({0.0f, -150.0f}, C);
        Vec2  r0 = {300.0f, 100.0f};
        Vec2  u  = {40.0f,  0.0f};   // enemy moving right
        float t0 = 0.0f;
        float tPrime = 1.5f;

        float ti = solveSimultaneousLabTime(r0, u, t0, tPrime, f);

        // Verify: toShipFrame of (r(ti), ti) should give t' == tPrime
        Vec2  ri = r0 + u * ti;
        Event labEvent = { ri, ti };
        Event shipEvent = toShipFrame(labEvent, f);
        CHECK_NEAR(shipEvent.t, tPrime);
    }

    // --- contractOffset: zero contraction at beta=0 ---
    {
        BoostFrame f = BoostFrame::fromVelocity({0.0f, 0.0f}, C);
        Vec2 offset = {15.0f, 10.0f};
        Vec2 contracted = contractOffset(offset, f);
        CHECK_NEAR(contracted.x, offset.x);
        CHECK_NEAR(contracted.y, offset.y);
    }

    // --- contractOffset: parallel component shrinks by 1/gamma ---
    {
        // Boost purely in y direction; an offset purely in y should shrink.
        BoostFrame f = BoostFrame::fromVelocity({0.0f, -240.0f}, C);
        Vec2 offset = {0.0f, 20.0f};
        Vec2 contracted = contractOffset(offset, f);
        CHECK_NEAR(contracted.x, 0.0f);
        CHECK_NEAR(contracted.y, 20.0f / f.gamma);
    }

    return true;
}
