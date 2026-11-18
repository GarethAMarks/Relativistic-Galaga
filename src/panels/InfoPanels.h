#pragma once
#include "PanelKit.h"

// Click-through viewer for a deck of info panels.
class InfoPanels {
public:
    void open(const panels::Deck& deck) { deck_ = &deck; page_ = 0; }

    // Handle input.  Returns false when the player leaves the panels.
    bool update();
    void draw(const Assets& assets) const;

private:
    const panels::Deck* deck_ = nullptr;
    int page_ = 0;
};

// The decks (defined in their own files).
const panels::Deck& howToPlayDeck();
const panels::Deck& scienceDeck();
