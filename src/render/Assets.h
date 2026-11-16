#pragma once
#include <raylib.h>

// Holds one Texture2D per sprite.  id == 0 means the file was not found;
// callers must fall back to placeholder geometry in that case.
struct Assets {
    Texture2D ship         = {};
    Texture2D drone        = {};
    Texture2D fighter      = {};
    Texture2D heavy        = {};
    Texture2D enemy        = {};   // legacy formation enemy
    Texture2D boltPlayer   = {};   // greyscale: tinted at draw time (Doppler)
    Texture2D boltEnemy    = {};   // greyscale
    Texture2D boltHeavy    = {};   // greyscale
    Texture2D fxFlash      = {};   // explosion star-burst flash (greyscale)
    Texture2D glow         = {};   // procedural soft radial glow (white, alpha falloff)
    Texture2D ring         = {};   // procedural soft ring (shockwave)
    Texture2D powerupBoost = {};
    Texture2D powerupHeal  = {};
    Texture2D powerupTwin  = {};
    Texture2D fxHitEnemy   = {};   // impact burst: player bolt hits an enemy
    Texture2D fxHitPlayer  = {};   // impact burst: enemy bolt hits the player

    // Call once after InitWindow().  Missing PNG files are silently skipped.
    void load();
    // Call once before CloseWindow().
    void unload();
};
