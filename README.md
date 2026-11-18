# Relativistic Galaga

An arcade shooter for teaching special relativity, written in C++17 with [raylib](https://www.raylib.com/).

## Building

Requirements:

- CMake ≥ 3.22
- A C++17 compiler
- git (CMake downloads raylib 5.0 the first time it configures the project)

On Debian/Ubuntu, raylib also needs the X11/OpenGL development headers:

```sh
sudo apt install build-essential cmake git libx11-dev libxrandr-dev \
    libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev
```

Build:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Run the game and the unit tests:

```sh
./build/RelGalaga
./build/test_relativity
```

The asset path is fixed when CMake configures the project, so if you move the
source directory, run the configure step again.

Developed and tested on Linux (Ubuntu 22.04, GCC 11).

## Controls

| Key | Action |
|---|---|
| Arrow keys | Move |
| Space | Fire |
| `[` / `]` | Decrease / increase the in-game speed of light |
| M | Toggle music |
| Esc | Back to title / quit |

## Layout

```
src/
  main.cpp      app states (title, play, name entry) and main loop
  relativity/   Lorentz transform, aberration, Doppler (no raylib dependency)
  sim/          game state and logic: ship, enemies, bolts, powerups, spawning
  render/       drawing, sprite loading, starfield
  hud/          on-screen readouts
  audio/        sound effects and the procedurally generated music
  panels/       click-through "How to Play" and "The Science" screens
tests/          unit tests for src/relativity
assets/         sprites and sound effects
```

## Credits and licence

Code is released under the [MIT License](LICENSE).

Sprites and sound effects in `assets/` are from Kenney's
[Space Shooter Redux](https://kenney.nl/assets/space-shooter-redux) pack
(CC0; see `assets/LICENSE_kenney_space_shooter_redux.txt`).
raylib is distributed under the zlib/libpng licence.
