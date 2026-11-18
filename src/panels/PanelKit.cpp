#include "PanelKit.h"
#include "render/DrawUtil.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>

using namespace render;

namespace panels {

// ---- Text ---------------------------------------------------------------------

void text(const char* s, float x, float y, int size, Color col) {
    DrawText(s, (int)x, (int)y, size, col);
}

void textCentred(const char* s, float x, float y, int size, Color col) {
    DrawText(s, (int)(x - MeasureText(s, size) / 2.0f), (int)y, size, col);
}

void wrapped(const char* s, int x, int y, int width, int size, int lineH, Color col) {
    std::istringstream in(s);
    std::string word, line;
    while (in >> word) {
        std::replace(word.begin(), word.end(), '~', ' ');
        std::string trial = line.empty() ? word : line + " " + word;
        if (!line.empty() && MeasureText(trial.c_str(), size) > width) {
            DrawText(line.c_str(), x, y, size, col);
            y += lineH;
            line = word;
        } else {
            line = trial;
        }
    }
    if (!line.empty()) DrawText(line.c_str(), x, y, size, col);
}

void footnote(const Fig& f, const char* s, int size, Color col) {
    float y = f.box.y + f.box.height - 28.0f;
    DrawRectangle((int)f.box.x, (int)y, (int)f.box.width, 28, withAlpha(PANEL_BG, 0.9f));
    textCentred(s, f.c.x, y + (28 - size) / 2.0f, size, col);
}

// ---- Lines and shapes -------------------------------------------------------

void arrow(Vec2 from, Vec2 to, Color col, float thick) {
    Vec2  d = to - from;
    float L = d.length();
    if (L < 2.0f) return;
    Vec2  u = d / L, p = { -u.y, u.x };
    float head = std::min(11.0f, 0.5f * L);
    Vec2  base = to - u * head;
    DrawLineEx(rv(from), rv(base), thick, col);
    drawTriangleAny(to, base + p * 6.0f, base - p * 6.0f, col);
}

void dashed(Vec2 a, Vec2 b, Color col, float dash, float gap, float thick) {
    Vec2  d = b - a;
    float L = d.length();
    if (L < 1.0f) return;
    Vec2 u = d / L;
    for (float s = 0.0f; s < L; s += dash + gap)
        DrawLineEx(rv(a + u * s), rv(a + u * std::min(L, s + dash)), thick, col);
}

void dashedCircle(Vec2 c, float r, Color col) {
    const int n = 48;
    for (int i = 0; i < n; i += 2) {
        float t0 = 6.2831853f * i / n, t1 = 6.2831853f * (i + 1) / n;
        DrawLineEx({ c.x + r * std::cos(t0), c.y + r * std::sin(t0) },
                   { c.x + r * std::cos(t1), c.y + r * std::sin(t1) }, 1.5f, col);
    }
}

static void keyBody(Rectangle r) {
    Rectangle shadow = { r.x, r.y + 3.0f, r.width, r.height };
    DrawRectangleRounded(shadow, 0.25f, 6, Color{ 20, 22, 36, 255 });
    DrawRectangleRounded(r, 0.25f, 6, Color{ 48, 54, 84, 255 });
    DrawRectangleRoundedLines(r, 0.25f, 6, 1.5f, Color{ 130, 140, 180, 255 });
}

void keycap(Rectangle r, const char* label, int size) {
    keyBody(r);
    textCentred(label, r.x + r.width / 2.0f, r.y + (r.height - size) / 2.0f, size, WHITE);
}

void arrowKey(Rectangle r, Vec2 dir) {
    keyBody(r);
    Vec2  c = { r.x + r.width / 2.0f, r.y + r.height / 2.0f };
    Vec2  p = { -dir.y, dir.x };
    float s = 0.25f * std::min(r.width, r.height);
    drawTriangleAny(c + dir * s, c - dir * s + p * s, c - dir * s - p * s, WHITE);
}

// ---- Game sprites -----------------------------------------------------------

void ship(const Fig& f, Vec2 p, const BoostFrame* b, Color tint) {
    drawSpriteQuad(f.a.ship, p, 13.0f, 26.0f, 0.0f, b, tint);
}

void drone(const Fig& f, Vec2 p, Color tint, const BoostFrame* b) {
    drawSpriteQuad(f.a.drone, p, 20.0f, 18.0f, 0.0f, b, tint);
}

void fighter(const Fig& f, Vec2 p, const BoostFrame* b) {
    drawSpriteQuad(f.a.fighter, p, 24.0f, 19.0f, 0.0f, b, FIGHTER_COL);
}

void bolt(const Fig& f, Vec2 p, Vec2 dir, Color col) {
    float ang = headingAngle(dir);
    BeginBlendMode(BLEND_ADDITIVE);
    drawSpriteQuad(f.a.boltPlayer, p, 3.5f, 15.0f, ang, nullptr, col);
    drawSpriteQuad(f.a.boltPlayer, p, 1.6f, 14.0f, ang, nullptr, lerpColor(col, WHITE, 0.6f));
    EndBlendMode();
}

void glowDot(const Fig& f, Vec2 p, float r, Color col) {
    BeginBlendMode(BLEND_ADDITIVE);
    drawSpriteQuad(f.a.glow, p, r, r, 0.0f, nullptr, col);
    EndBlendMode();
}

void spriteFit(const Texture2D& tex, Vec2 p, float radius, Color tint) {
    float hw = radius, hh = radius;
    if (tex.id > 0) {
        float m = (float)std::max(tex.width, tex.height);
        hw = radius * tex.width / m;
        hh = radius * tex.height / m;
    }
    drawSpriteQuad(tex, p, hw, hh, 0.0f, nullptr, tint);
}

// ---- Whole-panel pieces -----------------------------------------------------

void drawFigure(const Assets& a, float x, const char* caption, FigureFn fn) {
    Rectangle box = { x, BOX_Y, BOX_W, BOX_H };
    DrawRectangleRec(box, PANEL_BG);
    BeginScissorMode((int)box.x, (int)box.y, (int)box.width, (int)box.height);
    Fig f{ a, box, { x + BOX_W / 2.0f, BOX_Y + 14.0f + BOX_H / 2.0f } };
    fn(f);
    // Caption on a strip so figure content never obscures it.
    DrawRectangle((int)box.x, (int)box.y, (int)box.width, 28, withAlpha(PANEL_BG, 0.9f));
    text(caption, box.x + 10.0f, box.y + 6.0f, 18, LABEL_COL);
    EndScissorMode();
    DrawRectangleLinesEx(box, 1.5f, PANEL_EDGE);
}

bool button(Rectangle r, const char* label, bool enabled) {
    bool hover = enabled && CheckCollisionPointRec(GetMousePosition(), r);
    Color edge = !enabled ? Color{ 50, 50, 60, 255 } : hover ? YELLOW : PANEL_EDGE;
    DrawRectangleRec(r, hover ? Color{ 40, 40, 20, 255 } : PANEL_BG);
    DrawRectangleLinesEx(r, 1.5f, edge);
    textCentred(label, r.x + r.width / 2.0f, r.y + 7.0f, 18,
                enabled ? (hover ? YELLOW : LABEL_COL) : Color{ 70, 70, 80, 255 });
    return hover;
}

} // namespace panels
