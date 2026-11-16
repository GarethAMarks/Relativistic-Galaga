#pragma once
#include "relativity/BoostFrame.h"

class Simulation;

class Hud {
public:
    void draw(const Simulation& sim,
              const BoostFrame& boost,
              bool  lorentzianMode,
              float c_game) const;
};
