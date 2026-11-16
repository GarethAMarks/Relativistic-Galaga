#include "Audio.h"
#include "Synth.h"
#include <atomic>
#include <cstdlib>
#include <string>

#ifndef ASSETS_DIR
#define ASSETS_DIR "assets/"
#endif

// ---- SoundPool --------------------------------------------------------------

void SoundPool::load(Sound s, int nVoices) {
    if (!IsSoundReady(s)) return;
    voices.push_back(s);
    for (int i = 1; i < nVoices; ++i) voices.push_back(LoadSoundAlias(s));
}

void SoundPool::play(float volume, float pitch, float pan) {
    if (voices.empty()) return;
    Sound& s = voices[next];
    next = (next + 1) % (int)voices.size();
    StopSound(s);
    SetSoundVolume(s, volume);
    SetSoundPitch(s, pitch);
    SetSoundPan(s, pan);
    PlaySound(s);
}

void SoundPool::unload() {
    for (size_t i = 1; i < voices.size(); ++i) UnloadSoundAlias(voices[i]);
    if (!voices.empty()) UnloadSound(voices[0]);
    voices.clear();
}

// ---- Helpers ----------------------------------------------------------------

static Sound loadFile(const char* name) {
    std::string path = std::string(ASSETS_DIR) + name;
    return FileExists(path.c_str()) ? LoadSound(path.c_str()) : Sound{};
}

static Sound fromPcm(const std::vector<short>& pcm) {
    Wave w = {};
    w.frameCount = (unsigned int)pcm.size();
    w.sampleRate = synth::RATE;
    w.sampleSize = 16;
    w.channels   = 1;
    w.data       = (void*)pcm.data();
    return LoadSoundFromWave(w);   // copies the samples
}

// Stereo position from lab-frame x.  (raylib 5.0: pan 1.0 = full left.)
static float panFor(Vec2 pos) {
    return 0.5f - 0.6f * (pos.x / 800.0f - 0.5f);
}

// Small random pitch variation so repeated sounds don't machine-gun.
static float jitter(float amount) {
    return 1.0f + amount * ((rand() % 2001) / 1000.0f - 1.0f);
}

// ---- Music stream -------------------------------------------------------------
// raylib's audio callback has no user pointer, so the loop lives in statics.

static std::vector<short>  g_musicPcm;
static std::atomic<size_t> g_musicPos{0};

static void musicCallback(void* out, unsigned int frames) {
    auto* dst = static_cast<short*>(out);
    size_t n = g_musicPcm.size(), pos = g_musicPos.load();
    for (unsigned int i = 0; i < frames; ++i) {
        dst[i] = g_musicPcm[pos];
        if (++pos >= n) pos = 0;
    }
    g_musicPos.store(pos);
}

// ---- Audio ------------------------------------------------------------------

void Audio::load() {
    ready_ = IsAudioDeviceReady();
    if (!ready_) return;

    firePlayer_.load(loadFile("sfx_fire_player.ogg"), 4);
    fireEnemy_ .load(loadFile("sfx_fire_enemy.ogg"),  6);
    hitEnemy_  .load(loadFile("sfx_hit_enemy.ogg"),   4);
    hitPlayer_ .load(loadFile("sfx_hit_player.ogg"),  2);
    powerup_   .load(loadFile("sfx_powerup.ogg"),     2);
    gameOver_  .load(loadFile("sfx_game_over.ogg"),   1);

    boomSmall_.load(fromPcm(synth::explosion({0.7f, 0.16f, 2600.0f, 250.0f,
                                              95.0f, 0.9f, 0.4f, 7u})), 6);
    boomBig_  .load(fromPcm(synth::explosion({1.5f, 0.38f, 1600.0f, 120.0f,
                                              70.0f, 1.4f, 1.0f, 99u})), 3);

    g_musicPcm = synth::musicLoop();
    g_musicPos = 0;
    music_ = LoadAudioStream(synth::RATE, 16, 1);
    SetAudioStreamCallback(music_, musicCallback);
    applyMusicVolume();
    PlayAudioStream(music_);
}

void Audio::unload() {
    if (!ready_) return;
    StopAudioStream(music_);
    UnloadAudioStream(music_);
    for (SoundPool* p : { &firePlayer_, &fireEnemy_, &hitEnemy_, &hitPlayer_,
                          &powerup_, &gameOver_, &boomSmall_, &boomBig_ })
        p->unload();
    ready_ = false;
}

void Audio::setMusicVolume(float v) {
    if (v == musicVolume_) return;
    musicVolume_ = v;
    applyMusicVolume();
}

void Audio::applyMusicVolume() {
    if (ready_) SetAudioStreamVolume(music_, musicOn_ ? musicVolume_ : 0.0f);
}

void Audio::playGameOver() {
    if (ready_) gameOver_.play(0.6f);
}

void Audio::handle(const std::vector<SimEvent>& events) {
    if (!ready_) return;
    int enemyFireSounds = 0;   // cap per frame: many enemies can fire at once
    for (const SimEvent& e : events) {
        float pan = panFor(e.pos);
        switch (e.type) {
        case SimEventType::PlayerFired:
            firePlayer_.play(0.30f, jitter(0.03f), pan);
            break;
        case SimEventType::EnemyFired: {
            if (enemyFireSounds++ >= 2) break;
            float pitch = (e.enemy == EnemyKind::Heavy)   ? 0.65f
                        : (e.enemy == EnemyKind::Fighter) ? 1.15f : 0.9f;
            float vol   = (e.enemy == EnemyKind::Heavy)   ? 0.22f : 0.12f;
            fireEnemy_.play(vol, pitch * jitter(0.04f), pan);
            break;
        }
        case SimEventType::EnemyHit: {
            float pitch = (e.enemy == EnemyKind::Heavy) ? 0.7f : 1.0f;
            hitEnemy_.play(0.30f, pitch * jitter(0.05f), pan);
            break;
        }
        case SimEventType::EnemyDestroyed:
            if (e.enemy == EnemyKind::Heavy) boomBig_.play(0.75f, jitter(0.05f), pan);
            else boomSmall_.play(0.55f, (e.enemy == EnemyKind::Fighter ? 0.9f : 1.05f)
                                         * jitter(0.06f), pan);
            break;
        case SimEventType::PlayerHit:
            hitPlayer_.play(0.6f, 1.0f, pan);
            break;
        case SimEventType::PlayerDestroyed:
            boomBig_.play(0.9f, 0.85f, pan);
            break;
        case SimEventType::PowerupCollected:
            powerup_.play(0.5f, 1.0f, pan);
            break;
        }
    }
}
