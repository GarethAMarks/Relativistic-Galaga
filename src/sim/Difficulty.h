#pragma once
#include <algorithm>

// Difficulty curve as a function of elapsed lab time.  All the tuning knobs
// for how the game ramps up live here.
namespace difficulty {

// Spawn interval is drawn uniformly from [MIN, MAX] at t = 0, then divided by
// (1 + RAMP * t / RAMP_TIME), floored at FLOOR.
constexpr float SPAWN_MIN       = 1.0f;
constexpr float SPAWN_MAX       = 5.0f;
constexpr float SPAWN_RAMP      = 0.35f;
constexpr float SPAWN_RAMP_TIME = 60.0f;   // s
constexpr float SPAWN_FLOOR     = 0.35f;   // s

inline float smoothstep(float a, float b, float x) {
    float t = std::clamp((x - a) / (b - a), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

// Multiplier applied to the base spawn interval.
inline float spawnIntervalScale(float t) {
    return 1.0f / (1.0f + SPAWN_RAMP * t / SPAWN_RAMP_TIME);
}

// Relative spawn weights for each enemy type at lab time t.
// Fighters start appearing after ~20 s, heavies after ~60 s; drones thin out.
inline float droneWeight(float t)   { return std::max(0.5f, 1.0f - t / 600.0f); }
inline float fighterWeight(float t) { return 1.2f * smoothstep(20.0f, 140.0f, t); }
inline float heavyWeight(float t)   { return 0.9f * smoothstep(60.0f, 300.0f, t); }

// Displayed "threat level", 1 + one level per 45 s.
inline int threatLevel(float t) { return 1 + static_cast<int>(t / 45.0f); }

} // namespace difficulty
