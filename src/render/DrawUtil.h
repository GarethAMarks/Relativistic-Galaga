#pragma once
#include <raylib.h>
#include "relativity/BoostFrame.h"

// Drawing helpers shared by the game renderer and the science panels.
namespace render {

inline Vector2 rv(Vec2 v) { return { v.x, v.y }; }

Color lerpColor(Color a, Color b, float t);
Color withAlpha(Color c, float a);

// Offset in an object's rest frame → offset as drawn (contracted if boosted).
Vec2 shapeOffset(Vec2 o, const BoostFrame* boost);

// Rotation that points a sprite's "up" (-y) along `dir`.
float headingAngle(Vec2 dir);

// Filled triangle; vertex order is fixed up so it is never back-face culled.
void drawTriangleAny(Vec2 a, Vec2 b, Vec2 c, Color col);

// Draws a (hw x hh)-half-extent sprite centred on `centre`, rotated by `angle`
// radians, and — if `boost` is non-null — Lorentz-contracted along boost.n by
// 1/gamma.  Rotation is applied to the rest-frame shape first, then the
// contraction, so the result is the exact parallelogram from contractOffset.
// Fallback when the texture is missing: a solid coloured quad.
void drawSpriteQuad(const Texture2D& tex, Vec2 centre, float hw, float hh,
                    float angle, const BoostFrame* boost, Color tint);

} // namespace render
