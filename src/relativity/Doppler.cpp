#include "Doppler.h"
#include <algorithm>

namespace relativity {

// D = 1 / (gamma * (1 - beta * cos(theta_obs)))
// Special cases:
//   theta=0   (ahead):     D = sqrt((1+beta)/(1-beta))  blueshift
//   theta=pi  (behind):    D = sqrt((1-beta)/(1+beta))  redshift
//   theta=pi/2 (sideways): D = 1/gamma                  transverse, purely relativistic
float dopplerFactor(float cosThetaObs, const BoostFrame& f) {
    float denom = f.gamma * (1.0f - f.beta * cosThetaObs);
    // Guard against divide-by-zero (only possible if beta=1, which we prevent)
    denom = std::max(denom, 1e-6f);
    return 1.0f / denom;
}

} // namespace relativity
