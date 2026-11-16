#include "relativity/Doppler.h"
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

bool testDoppler() {
    using namespace relativity;

    const float beta = 0.6f;
    BoostFrame f = BoostFrame::fromVelocity({0.0f, -beta * C}, C);

    // --- Head-on (theta=0): D = sqrt((1+beta)/(1-beta)) ---
    {
        float expected = std::sqrt((1.0f + beta) / (1.0f - beta));
        float got = dopplerFactor(1.0f, f);
        CHECK_NEAR(got, expected);
    }

    // --- Receding (theta=pi): D = sqrt((1-beta)/(1+beta)) ---
    {
        float expected = std::sqrt((1.0f - beta) / (1.0f + beta));
        float got = dopplerFactor(-1.0f, f);
        CHECK_NEAR(got, expected);
    }

    // --- Transverse (theta=pi/2): D = 1/gamma ---
    {
        float expected = 1.0f / f.gamma;
        float got = dopplerFactor(0.0f, f);
        CHECK_NEAR(got, expected);
    }

    // --- D = 1 when beta = 0 ---
    {
        BoostFrame f0 = BoostFrame::fromVelocity({0.0f, 0.0f}, C);
        CHECK_NEAR(dopplerFactor(1.0f, f0), 1.0f);
        CHECK_NEAR(dopplerFactor(0.0f, f0), 1.0f);
    }

    // --- Zero proper time along a light-speed trajectory ---
    // For a particle advancing at |u|=c per dt of lab time, the spacetime
    // interval ds^2 = -c^2 dt^2 + |u|^2 dt^2 = 0, so dτ = 0.
    {
        float accumulatedProperTime = 0.0f;
        float labDt = 0.01f;
        for (int i = 0; i < 100; ++i) {
            // Velocity magnitude = c
            float ux = 0.0f, uy = -C;  // |u| = C exactly
            float ds2 = -(C * C) * (labDt * labDt)
                        + (ux * ux + uy * uy) * (labDt * labDt);
            // dτ = sqrt(-ds²)/c  (timelike sign convention).
            // ds2 should be 0, so dτ = 0.
            float dtau = (ds2 <= 0.0f) ? std::sqrt(-ds2) / C : 0.0f;
            accumulatedProperTime += dtau;
        }
        CHECK_NEAR(accumulatedProperTime, 0.0f);
    }

    return true;
}
