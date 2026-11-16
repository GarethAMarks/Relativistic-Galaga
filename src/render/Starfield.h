#pragma once
#include "relativity/BoostFrame.h"
#include <array>

struct Star {
    float x, y;
    float brightness;  // 0..1 mapped to alpha
};

class Starfield {
public:
    Starfield();

    // Scroll stars opposite to ship motion for basic parallax (Newtonian mode).
    // In Lorentzian mode parallax is disabled; aberration is applied at draw time.
    void update(Vec2 shipVel, float dt, bool lorentzianMode);
    void draw(const BoostFrame& boost, bool lorentzianMode, Vec2 shipPos) const;

private:
    static constexpr int N = 600;
    std::array<Star, N> stars_;
};
