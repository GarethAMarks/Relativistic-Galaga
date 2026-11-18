#include "InfoPanels.h"
#include <raylib.h>

using namespace panels;

static const Rectangle BACK_BTN = { 30.0f,  556.0f, 110.0f, 32.0f };
static const Rectangle NEXT_BTN = { 660.0f, 556.0f, 110.0f, 32.0f };

bool InfoPanels::update() {
    if (!deck_ || IsKeyPressed(KEY_ESCAPE)) return false;

    bool next = IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
    bool prev = IsKeyPressed(KEY_LEFT)  || IsKeyPressed(KEY_BACKSPACE);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        // Back button goes back; a click anywhere else advances.
        if (CheckCollisionPointRec(GetMousePosition(), BACK_BTN)) prev = true;
        else next = true;
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) prev = true;

    if (prev && page_ > 0) --page_;
    if (next) {
        if (page_ == deck_->count - 1) return false;   // "Done"
        ++page_;
    }
    return true;
}

void InfoPanels::draw(const Assets& assets) const {
    if (!deck_) return;
    const Panel& p = deck_->panels[page_];
    const int n = deck_->count;

    text(deck_->header, 30.0f, 22.0f, 16, YELLOW);
    text(p.title, 30.0f + MeasureText(deck_->header, 16) + 16.0f, 16.0f, 28, WHITE);

    drawFigure(assets, BOX_LEFT_X,  p.captionA, p.drawA);
    drawFigure(assets, BOX_RIGHT_X, p.captionB, p.drawB);

    wrapped(p.body, 32, TEXT_Y, SCREEN_W - 64, TEXT_SIZE, TEXT_LINE, Color{ 225, 228, 240, 255 });

    // Navigation
    button(BACK_BTN, "< Back", page_ > 0);
    button(NEXT_BTN, page_ == n - 1 ? "Done" : "Next >", true);
    for (int i = 0; i < n; ++i) {
        float x = SCREEN_W / 2.0f + (i - (n - 1) / 2.0f) * 18.0f;
        DrawCircleV({ x, 566.0f }, i == page_ ? 5.0f : 3.5f,
                    i == page_ ? YELLOW : Color{ 90, 95, 120, 255 });
    }
    textCentred("click or arrow keys to move   |   Esc: menu", SCREEN_W / 2.0f, 578.0f,
                12, Color{ 110, 115, 140, 255 });
}
