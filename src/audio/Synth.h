#pragma once
#include <vector>

// Tiny offline synthesiser: renders sound effects and the music loop into
// 16-bit mono PCM at startup, so no audio files are needed for them.
namespace synth {

constexpr int RATE = 44100;

struct ExplosionParams {
    float    duration;      // s
    float    decay;         // s, amplitude envelope time constant
    float    cutoffStart;   // Hz, low-pass on the noise at t = 0 ...
    float    cutoffEnd;     // ... sweeping down to this
    float    thumpHz;       // initial pitch of the low "boom" sine
    float    thumpGain;
    float    crackle;       // density of crackle impulses (0 = none)
    unsigned seed;
};

// Filtered-noise explosion with a pitch-dropping thump and optional crackle.
std::vector<short> explosion(const ExplosionParams& p);

// Seamlessly looping chiptune backing track (A minor, ~132 BPM, 8 bars):
// triangle octave bass, pulse-wave arpeggio, square lead, noise/sine drums.
std::vector<short> musicLoop();

} // namespace synth
