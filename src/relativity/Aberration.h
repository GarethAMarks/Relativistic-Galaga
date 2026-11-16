#pragma once
#include "BoostFrame.h"

namespace relativity {

// Eq. 8: relativistic aberration of a unit direction vector.
// toShip=true  -> convert lab-frame direction to ship-frame direction
// toShip=false -> convert ship-frame direction to lab-frame direction
// Used in exactly two places: (1) firing direction, (2) starfield skybox.
Vec2 aberrate(Vec2 unitDir, const BoostFrame& f, bool toShip);

} // namespace relativity
