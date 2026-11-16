#pragma once
#include "sim/SimEvent.h"
#include <raylib.h>
#include <vector>

// A sound plus a few aliases sharing its sample data, so several copies can
// overlap (round-robin; the oldest voice is stolen when all are busy).
struct SoundPool {
    std::vector<Sound> voices;   // voices[0] owns the data
    int next = 0;

    void load(Sound s, int nVoices);
    void play(float volume, float pitch = 1.0f, float pan = 0.5f);
    void unload();
};

// Owns all game audio.  Reacts to Simulation events; plays the music loop.
// If no audio device is available every call is a harmless no-op.
class Audio {
public:
    // Call after InitAudioDevice().
    void load();
    void unload();

    void handle(const std::vector<SimEvent>& events);
    void playGameOver();

    void setMusicVolume(float v);           // 0..1, before the mute switch
    void toggleMusic() { musicOn_ = !musicOn_; applyMusicVolume(); }
    bool musicOn() const { return musicOn_; }

private:
    bool ready_ = false;

    SoundPool firePlayer_, fireEnemy_, hitEnemy_, hitPlayer_;
    SoundPool powerup_, gameOver_, boomSmall_, boomBig_;

    AudioStream music_   = {};
    float musicVolume_   = 0.3f;
    bool  musicOn_       = true;

    void applyMusicVolume();
};
