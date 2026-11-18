#include <raylib.h>
#include "sim/Simulation.h"
#include "relativity/BoostFrame.h"
#include "render/Assets.h"
#include "render/Renderer.h"
#include "render/Starfield.h"
#include "hud/Hud.h"
#include "audio/Audio.h"
#include "panels/InfoPanels.h"
#include <algorithm>
#include <string>
#include <vector>

enum class AppState { TITLE, PLAYING, ENTERING_NAME, PANELS };

struct LeaderEntry {
    std::string name;
    int  score;
    bool newtonian;   // run was played in Newtonian comparison mode
};

// Seconds the death explosion plays before the game-over screen.
static constexpr float DEATH_PAUSE = 1.6f;

int main() {
    InitWindow(800, 600, "Relativistic Galaga");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);   // ESC is handled per-state below
    InitAudioDevice();

    float     c_game          = 600.0f;
    bool      lorentzianMode  = true;   // fixed for the duration of a run
    int       menuChoice      = 0;      // 0 = Lorentzian, 1 = Newtonian, 2 = How to Play, 3 = The Science
    float     deathTimer      = 0.0f;
    AppState  state           = AppState::TITLE;

    Assets     assets;
    assets.load();
    Audio      audio;
    audio.load();

    Simulation sim(c_game);
    Renderer   renderer;
    Starfield  stars;
    Hud        hud;
    InfoPanels infoPanels;   // "How to Play" / "The Science"

    std::vector<LeaderEntry> leaderboard;
    std::string nameInput;
    int pendingScore = 0;
    bool quit = false;

    while (!quit && !WindowShouldClose()) {
        float dt = GetFrameTime();

        // ----- Global controls (active in any state) -----
        if (IsKeyPressed(KEY_LEFT_BRACKET)  && c_game > 240.0f) c_game -= 40.0f;
        if (IsKeyPressed(KEY_RIGHT_BRACKET) && c_game < 1200.0f) c_game += 40.0f;
        // M toggles music (not while typing a name, where M is a letter).
        if (state != AppState::ENTERING_NAME && IsKeyPressed(KEY_M)) audio.toggleMusic();
        audio.setMusicVolume(state == AppState::PLAYING ? 0.32f : 0.18f);

        BeginDrawing();
        ClearBackground(BLACK);

        switch (state) {

        case AppState::TITLE: {
            constexpr int MENU_N = 4;
            const char* items[MENU_N] = { "Start",
                                          "Start  (Newtonian comparison)",
                                          "How to Play",
                                          "The Science" };
            const Color itemCol[MENU_N] = { YELLOW, SKYBLUE, Color{255, 190, 110, 255},
                                            Color{140, 230, 160, 255} };
            const int   itemSize[MENU_N] = { 24, 20, 20, 20 };
            auto itemY = [](int i) { return 172 + i * 36; };

            if (IsKeyPressed(KEY_UP))   menuChoice = (menuChoice + MENU_N - 1) % MENU_N;
            if (IsKeyPressed(KEY_DOWN)) menuChoice = (menuChoice + 1) % MENU_N;
            bool activate = IsKeyPressed(KEY_ENTER);
            if (IsKeyPressed(KEY_N)) { menuChoice = 1; activate = true; }
            // Mouse: hover selects, click activates.
            Vector2 mouse = GetMousePosition();
            for (int i = 0; i < MENU_N; ++i) {
                Rectangle r = { 76.0f, (float)itemY(i) - 4.0f,
                                (float)MeasureText(items[i], itemSize[i]) + 36.0f, 32.0f };
                if (CheckCollisionPointRec(mouse, r)) {
                    if (GetMouseDelta().x != 0.0f || GetMouseDelta().y != 0.0f) menuChoice = i;
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { menuChoice = i; activate = true; }
                }
            }
            if (IsKeyPressed(KEY_ESCAPE)) quit = true;
            if (activate) {
                if (menuChoice >= 2) {
                    infoPanels.open(menuChoice == 2 ? howToPlayDeck() : scienceDeck());
                    state = AppState::PANELS;
                } else {
                    lorentzianMode = (menuChoice == 0);
                    sim.reset(c_game);
                    deathTimer = 0.0f;
                    state = AppState::PLAYING;
                }
            }

            // Left column: game info + menu
            DrawText("RELATIVISTIC GALAGA",      80, 70, 36, YELLOW);
            DrawText("A special-relativity demo", 92, 116, 20, LIGHTGRAY);

            for (int i = 0; i < MENU_N; ++i) {
                bool sel = (i == menuChoice);
                if (sel) DrawText(">", 80, itemY(i), 24, itemCol[i]);
                DrawText(items[i], 104, itemY(i), itemSize[i], sel ? itemCol[i] : DARKGRAY);
            }
            DrawText("Up/Down or mouse: choose   ENTER: select", 92, 324, 16, GRAY);
            DrawText(TextFormat("c = %.0f px/s  ([ / ] to adjust)", c_game),
                     92, 352, 18, GREEN);

            DrawText("Arrows: move   Space: fire   Esc: quit", 92, 510, 18, DARKGRAY);
            DrawText(TextFormat("M: music %s", audio.musicOn() ? "on" : "off"),
                     92, 534, 16, DARKGRAY);

            // Right column: leaderboard (only when there are entries)
            if (!leaderboard.empty()) {
                DrawLine(430, 55, 430, 560, DARKGRAY);
                DrawText("HIGH SCORES", 510, 65, 22, YELLOW);
                int shown = std::min((int)leaderboard.size(), 10);
                for (int i = 0; i < shown; ++i) {
                    Color col = (i == 0) ? GOLD
                              : (i == 1) ? LIGHTGRAY
                              : (i == 2) ? ORANGE
                              : WHITE;
                    DrawText(TextFormat("%d.", i + 1),                    450, 106 + i * 44, 20, col);
                    DrawText(leaderboard[i].name.c_str(),                 480, 106 + i * 44, 20, col);
                    DrawText(TextFormat("%d", leaderboard[i].score),      680, 106 + i * 44, 20, col);
                    if (leaderboard[i].newtonian)
                        DrawText("N", 760, 110 + i * 44, 14, SKYBLUE);
                }
                if (std::any_of(leaderboard.begin(), leaderboard.end(),
                                [](const LeaderEntry& e){ return e.newtonian; }))
                    DrawText("N = Newtonian run", 600, 548, 12, DARKGRAY);
            }
            break;
        }

        case AppState::PLAYING: {
            if (IsKeyPressed(KEY_ESCAPE)) { state = AppState::TITLE; break; }

            sim.update(dt, c_game, lorentzianMode);
            audio.handle(sim.events());

            BoostFrame boost = BoostFrame::fromVelocity(sim.ship().vel, c_game);

            stars.update(sim.ship().vel, dt, lorentzianMode);
            stars.draw(boost, lorentzianMode, sim.ship().pos);
            renderer.draw(sim, boost, lorentzianMode, c_game, assets);
            hud.draw(sim, boost, lorentzianMode, c_game);

            if (sim.isGameOver()) {
                deathTimer += dt;
                if (deathTimer >= DEATH_PAUSE) {
                    pendingScore = sim.score();
                    nameInput.clear();
                    audio.playGameOver();
                    state = AppState::ENTERING_NAME;
                }
            }
            break;
        }

        case AppState::PANELS: {
            stars.draw(BoostFrame::fromVelocity({0.0f, 0.0f}, c_game), false, {400.0f, 300.0f});
            if (!infoPanels.update()) state = AppState::TITLE;
            else infoPanels.draw(assets);
            break;
        }

        case AppState::ENTERING_NAME: {
            // Drain the character buffer.
            int ch;
            while ((ch = GetCharPressed()) > 0) {
                if (ch >= 32 && ch <= 126 && (int)nameInput.size() < 12)
                    nameInput += static_cast<char>(ch);
            }
            if (IsKeyPressed(KEY_BACKSPACE) && !nameInput.empty())
                nameInput.pop_back();
            if (IsKeyPressed(KEY_ENTER)) {
                if (!nameInput.empty()) {
                    leaderboard.push_back({nameInput, pendingScore, !lorentzianMode});
                    std::sort(leaderboard.begin(), leaderboard.end(),
                              [](const LeaderEntry& a, const LeaderEntry& b){
                                  return a.score > b.score;
                              });
                    if ((int)leaderboard.size() > 10) leaderboard.resize(10);
                }
                nameInput.clear();
                state = AppState::TITLE;
            }
            if (IsKeyPressed(KEY_ESCAPE)) {
                nameInput.clear();
                state = AppState::TITLE;
            }

            // Draw game-over / name entry screen.
            DrawText("GAME  OVER",      220, 155, 60, RED);
            DrawText(TextFormat("Score: %d", pendingScore), 310, 245, 34, WHITE);

            DrawText("Enter your name:", 240, 322, 24, LIGHTGRAY);
            DrawRectangleLines(240, 360, 320, 46, GRAY);
            bool cursorOn = ((int)(GetTime() * 2) % 2 == 0);
            std::string display = nameInput + (cursorOn ? "|" : " ");
            DrawText(display.c_str(), 252, 373, 24, WHITE);

            DrawText("ENTER to save   ESC to skip", 228, 428, 20, DARKGRAY);
            break;
        }
        }

        EndDrawing();
    }

    audio.unload();
    CloseAudioDevice();
    assets.unload();
    CloseWindow();
    return 0;
}
