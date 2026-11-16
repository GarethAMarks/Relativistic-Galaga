#include "Lorentz.h"

namespace relativity {

// Forward transform: lab -> ship frame.
// r'_par = gamma*(r_par - |v|*t)
// r'_perp = r_perp
// t' = gamma*(t - v.dot(r)/c^2)
Event toShipFrame(Event e, const BoostFrame& f) {
    float rPar = e.r.dot(f.n);          // scalar projection onto boost axis
    Vec2  rPerp = e.r - f.n * rPar;     // perpendicular component (unchanged)
    float speed = f.v.length();

    float newRPar = f.gamma * (rPar - speed * e.t);
    float newT    = f.gamma * (e.t - f.v.dot(e.r) / (f.c * f.c));

    return { f.n * newRPar + rPerp, newT };
}

// Inverse transform: ship -> lab frame.
// r_par = gamma*(r'_par + |v|*t')
// r_perp = r'_perp
// t = gamma*(t' + v.dot(r')/c^2)
Event toLabFrame(Event e, const BoostFrame& f) {
    float rPar = e.r.dot(f.n);
    Vec2  rPerp = e.r - f.n * rPar;
    float speed = f.v.length();

    float newRPar = f.gamma * (rPar + speed * e.t);
    float newT    = f.gamma * (e.t + f.v.dot(e.r) / (f.c * f.c));

    return { f.n * newRPar + rPerp, newT };
}

// Eq. 9: closed-form simultaneity solve for a uniformly-moving object.
float solveSimultaneousLabTime(Vec2 r0, Vec2 u, float t0,
                               float tPrime, const BoostFrame& f) {
    float c2      = f.c * f.c;
    float vDotR0  = f.v.dot(r0);
    float vDotU   = f.v.dot(u);
    float numerator   = tPrime / f.gamma + vDotR0 / c2 - (vDotU / c2) * t0;
    float denominator = 1.0f - vDotU / c2;
    return numerator / denominator;
}

// Eq. 5: contract a sprite-corner offset along the boost axis.
// delta_r'_shape = delta_r_perp + (1/gamma)*delta_r_par
Vec2 contractOffset(Vec2 deltaR, const BoostFrame& f) {
    float dRPar  = deltaR.dot(f.n);
    Vec2  dRPerp = deltaR - f.n * dRPar;
    return dRPerp + f.n * (dRPar / f.gamma);
}

} // namespace relativity
