#include "DrawUtil.h"
#include "relativity/Lorentz.h"
#include <rlgl.h>
#include <algorithm>
#include <cmath>

namespace render {

Color lerpColor(Color a, Color b, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    auto l = [t](unsigned char x, unsigned char y) {
        return (unsigned char)(x + (y - x) * t);
    };
    return { l(a.r, b.r), l(a.g, b.g), l(a.b, b.b), l(a.a, b.a) };
}

Color withAlpha(Color c, float a) {
    return { c.r, c.g, c.b, (unsigned char)(255.0f * std::clamp(a, 0.0f, 1.0f)) };
}

Vec2 shapeOffset(Vec2 o, const BoostFrame* boost) {
    return boost ? relativity::contractOffset(o, *boost) : o;
}

float headingAngle(Vec2 dir) {
    return std::atan2(dir.x, -dir.y);
}

void drawTriangleAny(Vec2 a, Vec2 b, Vec2 c, Color col) {
    // raylib (y-down screen) draws triangles whose cross product is negative.
    float cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    if (cross > 0.0f) std::swap(b, c);
    DrawTriangle(rv(a), rv(b), rv(c), col);
}

void drawSpriteQuad(const Texture2D& tex, Vec2 centre, float hw, float hh,
                    float angle, const BoostFrame* boost, Color tint) {
    // Corners in offset-from-centre space: TL=0, TR=1, BR=2, BL=3
    const Vec2 offs[4] = { {-hw,-hh}, {+hw,-hh}, {+hw,+hh}, {-hw,+hh} };
    float cs = std::cos(angle), sn = std::sin(angle);
    Vec2 cv[4];
    for (int i = 0; i < 4; ++i) {
        Vec2 o = { offs[i].x * cs - offs[i].y * sn, offs[i].x * sn + offs[i].y * cs };
        cv[i] = centre + shapeOffset(o, boost);
    }

    if (tex.id > 0) {
        // Map texture (0,0)→TL, (1,0)→TR, (1,1)→BR, (0,1)→BL.
        rlSetTexture(tex.id);
        rlBegin(RL_QUADS);
            rlColor4ub(tint.r, tint.g, tint.b, tint.a);
            rlTexCoord2f(0.0f, 0.0f); rlVertex2f(cv[0].x, cv[0].y);  // TL
            rlTexCoord2f(0.0f, 1.0f); rlVertex2f(cv[3].x, cv[3].y);  // BL
            rlTexCoord2f(1.0f, 1.0f); rlVertex2f(cv[2].x, cv[2].y);  // BR
            rlTexCoord2f(1.0f, 0.0f); rlVertex2f(cv[1].x, cv[1].y);  // TR
        rlEnd();
        rlSetTexture(0);
    } else {
        drawTriangleAny(cv[0], cv[3], cv[2], tint);
        drawTriangleAny(cv[0], cv[2], cv[1], tint);
    }
}

} // namespace render
