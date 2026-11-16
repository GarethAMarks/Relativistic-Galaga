#pragma once
#include "BoostFrame.h"

namespace relativity {

// Eq. 10: relativistic Doppler factor D = f_obs / f_0.
// cosThetaObs = cos of angle between ship velocity direction n and the
// direction from ship to the source (or bolt flight direction for bolt tint).
// D > 1: blueshift (source ahead); D < 1: redshift (source behind).
float dopplerFactor(float cosThetaObs, const BoostFrame& f);

} // namespace relativity
