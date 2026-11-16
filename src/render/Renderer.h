#pragma once
#include "Assets.h"
#include "sim/Simulation.h"
#include "relativity/BoostFrame.h"

class Renderer {
public:
    void draw(const Simulation& sim,
              const BoostFrame& boost,
              bool lorentzianMode,
              float c_game,
              const Assets& assets) const;
};
