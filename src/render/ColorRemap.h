#pragma once
#include <raylib.h>

namespace render {

// Shift the hue of a base colour by k * ln(dopplerFactor) degrees in HSV space.
// D > 1 blueshifts (hue toward blue), D < 1 redshifts (hue toward red).
// k is a tunable artistic constant (default 60 degrees per unit log-doppler).
Color hueShiftFromDoppler(Color base, float dopplerFactor, float k = 60.0f);

} // namespace render
