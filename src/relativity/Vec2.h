#pragma once
#include <cmath>

struct Vec2 {
    float x = 0.0f, y = 0.0f;

    Vec2 operator+(Vec2 b) const { return {x + b.x, y + b.y}; }
    Vec2 operator-(Vec2 b) const { return {x - b.x, y - b.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
    Vec2 operator/(float s) const { return {x / s, y / s}; }
    Vec2 operator-()        const { return {-x, -y}; }

    float dot(Vec2 b)  const { return x * b.x + y * b.y; }
    float lengthSq()   const { return x * x + y * y; }
    float length()     const { return std::sqrt(lengthSq()); }

    Vec2 normalized() const {
        float l = length();
        return l > 1e-9f ? *this / l : Vec2{0.0f, 0.0f};
    }
};

inline Vec2 operator*(float s, Vec2 v) { return v * s; }
