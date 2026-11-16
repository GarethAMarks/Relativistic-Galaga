#include "Synth.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace synth {

namespace {

constexpr float PI = 3.14159265358979f;

struct Rng {
    uint32_t s;
    explicit Rng(uint32_t seed) : s(seed ? seed : 1u) {}
    float next() {                       // uniform in [-1, 1)
        s ^= s << 13; s ^= s >> 17; s ^= s << 5;
        return (s & 0xFFFFFF) / 8388608.0f - 1.0f;
    }
};

std::vector<short> toPcm(const std::vector<float>& x, float peak) {
    float m = 1e-6f;
    for (float v : x) m = std::max(m, std::fabs(v));
    float g = peak / m;
    std::vector<short> out(x.size());
    for (size_t i = 0; i < x.size(); ++i)
        out[i] = static_cast<short>(std::clamp(x[i] * g, -1.0f, 1.0f) * 32767.0f);
    return out;
}

float midiHz(int note) { return 440.0f * std::pow(2.0f, (note - 69) / 12.0f); }

enum class Wave { Triangle, Pulse25, Square };

float osc(Wave w, float ph) {            // ph in [0,1)
    switch (w) {
        case Wave::Triangle: return 4.0f * std::fabs(ph - 0.5f) - 1.0f;
        case Wave::Pulse25:  return ph < 0.25f ? 1.0f : -1.0f;
        default:             return ph < 0.5f  ? 1.0f : -1.0f;
    }
}

// Mixes into a circular buffer so tails past the end wrap to the start,
// which makes the loop seamless.
struct LoopBuffer {
    std::vector<float> x;
    explicit LoopBuffer(size_t n) : x(n, 0.0f) {}
    void add(long i, float v) { x[(size_t)(i % (long)x.size())] += v; }
};

// A pitched note: attack, exponential decay towards `sustain`, release at gate end.
void addNote(LoopBuffer& buf, long start, float gateSec, float hz, Wave w,
             float amp, float decaySec, float sustain, float vibrato = 0.0f) {
    const float attack = 0.004f, release = 0.04f;
    long n = (long)((gateSec + release) * RATE);
    float ph = 0.0f;
    for (long i = 0; i < n; ++i) {
        float t   = (float)i / RATE;
        float env = (t < attack) ? t / attack
                  : sustain + (1.0f - sustain) * std::exp(-(t - attack) / decaySec);
        if (t > gateSec) env *= std::max(0.0f, 1.0f - (t - gateSec) / release);
        float f = hz * (1.0f + vibrato * std::sin(2.0f * PI * 5.5f * t) * std::min(1.0f, t / 0.25f));
        ph += f / RATE;
        ph -= std::floor(ph);
        buf.add(start + i, amp * env * osc(w, ph));
    }
}

void addKick(LoopBuffer& buf, long start, float amp) {
    long n = (long)(0.35f * RATE);
    float ph = 0.0f;
    for (long i = 0; i < n; ++i) {
        float t = (float)i / RATE;
        ph += (45.0f + 110.0f * std::exp(-t / 0.03f)) / RATE;
        buf.add(start + i, amp * std::exp(-t / 0.12f) * std::sin(2.0f * PI * ph));
    }
}

void addSnare(LoopBuffer& buf, long start, float amp, Rng& rng) {
    long n = (long)(0.25f * RATE);
    float lp = 0.0f, ph = 0.0f;
    for (long i = 0; i < n; ++i) {
        float t  = (float)i / RATE;
        float nz = rng.next();
        lp += 0.25f * (nz - lp);
        ph += 190.0f / RATE;
        float v = (nz - lp) * std::exp(-t / 0.06f)
                + 0.5f * std::sin(2.0f * PI * ph) * std::exp(-t / 0.04f);
        buf.add(start + i, amp * v);
    }
}

void addHat(LoopBuffer& buf, long start, float amp, float decay, Rng& rng) {
    long n = (long)(6.0f * decay * RATE);
    float lp = 0.0f;
    for (long i = 0; i < n; ++i) {
        float t  = (float)i / RATE;
        float nz = rng.next();
        lp += 0.6f * (nz - lp);
        buf.add(start + i, amp * (nz - lp) * std::exp(-t / decay));
    }
}

} // namespace

// ---------------------------------------------------------------------------

std::vector<short> explosion(const ExplosionParams& p) {
    Rng rng(p.seed);
    size_t n = (size_t)(p.duration * RATE);
    std::vector<float> x(n);
    float lp1 = 0.0f, lp2 = 0.0f, ph = 0.0f;
    for (size_t i = 0; i < n; ++i) {
        float t   = (float)i / RATE;
        float env = (1.0f - std::exp(-t / 0.003f)) * std::exp(-t / p.decay);
        float fc  = p.cutoffEnd + (p.cutoffStart - p.cutoffEnd) * std::exp(-t / (0.6f * p.decay));
        float a   = 1.0f - std::exp(-2.0f * PI * fc / RATE);
        float nz  = rng.next();
        lp1 += a * (nz - lp1);
        lp2 += a * (lp1 - lp2);                         // 2-pole: darker rumble

        ph += p.thumpHz * (0.35f + 0.65f * std::exp(-t / 0.08f)) / RATE;
        float thump = std::sin(2.0f * PI * ph) * std::exp(-t / 0.14f);

        float crack = 0.0f;
        if (p.crackle > 0.0f && std::fabs(rng.next()) < p.crackle * std::exp(-t / (0.5f * p.decay)) * 0.004f)
            crack = rng.next();

        x[i] = env * (3.0f * lp2 + p.thumpGain * thump) + 0.6f * crack * env;
    }
    // Short fade-out so the tail never clicks.
    size_t fade = std::min(n, (size_t)(0.02f * RATE));
    for (size_t i = 0; i < fade; ++i) x[n - 1 - i] *= (float)i / fade;
    return toPcm(x, 0.9f);
}

std::vector<short> musicLoop() {
    const float bpm  = 132.0f;
    const float step = 60.0f / bpm / 4.0f;          // one 16th note, s
    const int   STEPS_PER_BAR = 16, BARS = 8;
    const int   steps = STEPS_PER_BAR * BARS;
    LoopBuffer buf((size_t)(steps * step * RATE));
    auto at = [&](int s) { return (long)(s * step * RATE); };

    // Chord per pair of bars: Am – F – C – G.
    struct Chord { int root; bool minor; };
    const Chord chords[4] = { {45, true}, {41, false}, {48, false}, {43, false} };

    Rng rng(1234);
    for (int bar = 0; bar < BARS; ++bar) {
        const Chord& ch = chords[bar / 2];
        int third = ch.minor ? 3 : 4;
        int s0 = bar * STEPS_PER_BAR;

        // Bass: driving octave 8ths on triangle.
        for (int k = 0; k < 8; ++k) {
            int note = ch.root + ((k % 2) ? 12 : 0);
            addNote(buf, at(s0 + 2 * k), step * 1.7f, midiHz(note), Wave::Triangle,
                    0.55f, 0.12f, 0.5f);
        }

        // Arpeggio: 16ths up through the chord, two octaves above the bass.
        const int arp[4] = { 0, third, 7, 12 };
        for (int k = 0; k < 16; ++k) {
            int idx  = (bar % 2 == 0) ? k % 4 : 3 - k % 4;   // up, then down
            int note = ch.root + 24 + arp[idx];
            addNote(buf, at(s0 + k), step * 0.6f, midiHz(note), Wave::Pulse25,
                    0.07f, 0.05f, 0.0f);
        }

        // Drums.
        addKick(buf, at(s0 + 0), 0.8f);
        addKick(buf, at(s0 + 8), 0.8f);
        if (bar % 2 == 1) addKick(buf, at(s0 + 10), 0.6f);
        addSnare(buf, at(s0 + 4),  0.35f, rng);
        addSnare(buf, at(s0 + 12), 0.35f, rng);
        for (int k = 0; k < 16; k += 2)
            addHat(buf, at(s0 + k), (k % 4 == 2) ? 0.12f : 0.07f, 0.012f, rng);
    }

    // Lead melody: {start step, MIDI note, length in steps}.
    struct N { int s, note, len; };
    const N melody[] = {
        // Am
        {0,76,4}, {4,72,2}, {6,74,2}, {8,76,6}, {14,74,2},
        {16,72,4}, {20,69,4}, {24,72,4}, {28,71,4},
        // F
        {32,69,6}, {38,72,2}, {40,77,8}, {48,76,4}, {52,74,4}, {56,72,8},
        // C
        {64,72,4}, {68,76,4}, {72,79,6}, {78,77,2}, {80,76,4}, {84,74,4},
        {88,72,4}, {92,76,4},
        // G
        {96,74,6}, {102,71,2}, {104,74,4}, {108,79,4}, {112,76,4}, {116,74,4},
        {120,71,8},
    };
    for (const N& m : melody)
        addNote(buf, at(m.s), step * (m.len - 0.3f), midiHz(m.note), Wave::Square,
                0.10f, 0.25f, 0.6f, 0.006f);

    // Gentle low-pass to take the edge off the raw square waves.  Run the
    // filter around the loop twice so its state is continuous at the seam.
    std::vector<float>& x = buf.x;
    float a = 1.0f - std::exp(-2.0f * PI * 7000.0f / RATE), y = 0.0f;
    for (float v : x) y += a * (v - y);
    for (float& v : x) { y += a * (v - y); v = std::tanh(1.1f * y); }
    return toPcm(x, 0.8f);
}

} // namespace synth
