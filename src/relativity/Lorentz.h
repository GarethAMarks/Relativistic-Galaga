#pragma once
#include "BoostFrame.h"

namespace relativity {

struct Event { Vec2 r; float t; };

// Eqs. 2-4: transform a lab-frame event into the ship frame.
Event toShipFrame(Event e, const BoostFrame& f);

// Inverse of toShipFrame (swap primed/unprimed, v -> -v).
Event toLabFrame(Event e, const BoostFrame& f);

// Eq. 9: for an object at r0 with constant lab velocity u since t0,
// find the lab time whose ship-frame image has ship-time == tPrime.
float solveSimultaneousLabTime(Vec2 r0, Vec2 u, float t0,
                               float tPrime, const BoostFrame& f);

// Eq. 5: Lorentz-contract a sprite offset vector about an object's centre.
Vec2 contractOffset(Vec2 deltaR, const BoostFrame& f);

} // namespace relativity
