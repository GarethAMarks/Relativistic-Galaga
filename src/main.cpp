#include <raylib.h>
#include "sim/Simulation.h"
#include "relativity/BoostFrame.h"
#include "render/Assets.h"
#include "render/Renderer.h"
#include "render/Starfield.h"
#include "hud/Hud.h"
#include "audio/Audio.h"
#include <algorithm>
#include <string>
#include <vector>

enum class AppState { TITLE, PLAYING, ENTERING_NAME };

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
    int       menuChoice      = 0;      // 0 = Lorentzian, 1 = Newtonian
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
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN)) menuChoice ^= 1;
            bool start = false;
            if (IsKeyPressed(KEY_ENTER)) start = true;
            if (IsKeyPressed(KEY_N)) { menuChoice = 1; start = true; }
            if (IsKeyPressed(KEY_ESCAPE)) quit = true;
            if (start) {
                lorentzianMode = (menuChoice == 0);
                sim.reset(c_game);
                deathTimer = 0.0f;
                state = AppState::PLAYING;
            }

            // Left column: game info + mode menu
            DrawText("RELATIVISTIC GALAGA",      80, 70, 36, YELLOW);
            DrawText("A special-relativity demo", 92, 116, 20, LIGHTGRAY);

            const char* items[2] = { "Start  (Lorentzian)",
                                     "Start  (Newtonian, for comparison)" };
            const Color itemCol[2] = { YELLOW, SKYBLUE };
            for (int i = 0; i < 2; ++i) {
                int y = 180 + i * 40;
                bool sel = (i == menuChoice);
                if (sel) DrawText(">", 80, y, 24, itemCol[i]);
                DrawText(items[i], 104, y, i == 0 ? 24 : 20,
                         sel ? itemCol[i] : DARKGRAY);
            }
            DrawText("Up/Down: choose   ENTER: start", 92, 272, 16, GRAY);
            DrawText(TextFormat("c = %.0f px/s  ([ / ] to adjust)", c_game),
                     92, 304, 18, GREEN);

            DrawText("Enemies",                      92, 360, 18, LIGHTGRAY);
            DrawText("Drone    1 hit    20 pts",     92, 384, 16, SKYBLUE);
            DrawText("Fighter  2 hits   40 pts",     92, 404, 16, Color{255, 180, 140, 255});
            DrawText("Heavy    5 hits  100 pts",     92, 424, 16, Color{192, 140, 255, 255});

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
