#pragma once
#include "render/Assets.h"
#include "relativity/BoostFrame.h"
#include <raylib.h>

// Shared building blocks for the click-through info panels ("How to Play",
// "The Science"): layout constants, the panel/deck description, and drawing
// primitives for composing still figures from game assets.
namespace panels {

// ---- Layout -----------------------------------------------------------------
constexpr int   SCREEN_W    = 800;
constexpr float BOX_W       = 355.0f;
constexpr float BOX_H       = 255.0f;
constexpr float BOX_Y       = 62.0f;
constexpr float BOX_LEFT_X  = 30.0f;
constexpr float BOX_RIGHT_X = 415.0f;
constexpr int   TEXT_Y      = 334;
constexpr int   TEXT_SIZE   = 19;
constexpr int   TEXT_LINE   = 24;

// ---- Palette ----------------------------------------------------------------
inline const Color PANEL_BG    = { 10, 14, 30, 225 };
inline const Color PANEL_EDGE  = { 70, 80, 120, 255 };
inline const Color LABEL_COL   = { 200, 205, 220, 255 };
inline const Color ARROW_COL   = { 120, 230, 140, 255 };   // velocities
inline const Color GUIDE_COL   = { 110, 120, 160, 255 };   // dashed guides
inline const Color BOLT_COL    = YELLOW;
inline const Color DRONE_COL   = SKYBLUE;                  // as in the game
inline const Color FIGHTER_COL = { 255, 180, 140, 255 };   // as in the game

// ---- Panels and decks -------------------------------------------------------

// Context for drawing one figure.
struct Fig {
    const Assets& a;
    Rectangle     box;
    Vec2          c;   // centre of the drawable area (below the caption)
};

using FigureFn = void (*)(const Fig&);

// One page: a title, two captioned figures, and body text underneath.
// In body text '~' is a non-breaking space.
struct Panel {
    const char* title;
    const char* captionA;
    FigureFn    drawA;
    const char* captionB;
    FigureFn    drawB;
    const char* body;
};

struct Deck {
    const char*  header;   // small label before each panel title
    const Panel* panels;
    int          count;
};

// ---- Text ---------------------------------------------------------------------
void text(const char* s, float x, float y, int size, Color col);
void textCentred(const char* s, float x, float y, int size, Color col);
void wrapped(const char* s, int x, int y, int width, int size, int lineH, Color col);
// One-line note along the bottom of a figure, on its own background strip.
void footnote(const Fig& f, const char* s, int size, Color col);

// ---- Lines and shapes -------------------------------------------------------
void arrow(Vec2 from, Vec2 to, Color col, float thick = 3.0f);
void dashed(Vec2 a, Vec2 b, Color col, float dash = 6.0f, float gap = 5.0f, float thick = 1.5f);
void dashedCircle(Vec2 c, float r, Color col);
// A keyboard key with a text label (e.g. "M", "Esc", "SPACE").
void keycap(Rectangle r, const char* label, int size = 16);
// A keyboard arrow key; dir is the arrow direction (unit axis vector).
void arrowKey(Rectangle r, Vec2 dir);

// ---- Game sprites -----------------------------------------------------------
void ship(const Fig& f, Vec2 p, const BoostFrame* b = nullptr, Color tint = WHITE);
void drone(const Fig& f, Vec2 p, Color tint = DRONE_COL, const BoostFrame* b = nullptr);
void fighter(const Fig& f, Vec2 p, const BoostFrame* b = nullptr);
// Glowing player bolt as drawn in the game, pointing along `dir`.
void bolt(const Fig& f, Vec2 p, Vec2 dir, Color col = BOLT_COL);
void glowDot(const Fig& f, Vec2 p, float r, Color col);
// Sprite scaled so its longer side is 2*radius, aspect preserved.
void spriteFit(const Texture2D& tex, Vec2 p, float radius, Color tint);

// ---- Whole-panel pieces (used by InfoPanels) --------------------------------
void drawFigure(const Assets& a, float x, const char* caption, FigureFn fn);
// Draws a button; returns true if the mouse is over it (and it is enabled).
bool button(Rectangle r, const char* label, bool enabled);

} // namespace panels
