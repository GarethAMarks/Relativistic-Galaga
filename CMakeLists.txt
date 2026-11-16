cmake_minimum_required(VERSION 3.22)
project(RelGalaga CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch raylib 5.0
include(FetchContent)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(raylib)

# ----- Game executable -----
add_executable(RelGalaga
    src/main.cpp
    src/sim/Ship.cpp
    src/sim/Enemy.cpp
    src/sim/Drone.cpp
    src/sim/Fighter.cpp
    src/sim/Heavy.cpp
    src/sim/Bolt.cpp
    src/sim/Powerup.cpp
    src/sim/Powerups.cpp
    src/sim/Spawner.cpp
    src/sim/Simulation.cpp
    src/relativity/BoostFrame.cpp
    src/relativity/Lorentz.cpp
    src/relativity/Aberration.cpp
    src/relativity/Doppler.cpp
    src/render/Assets.cpp
    src/render/Renderer.cpp
    src/render/Starfield.cpp
    src/render/ColorRemap.cpp
    src/hud/Hud.cpp
    src/audio/Audio.cpp
    src/audio/Synth.cpp
)
target_include_directories(RelGalaga PRIVATE src)
target_link_libraries(RelGalaga PRIVATE raylib)
if(UNIX AND NOT APPLE)
    target_link_libraries(RelGalaga PRIVATE GL m dl pthread X11)
endif()
target_compile_definitions(RelGalaga PRIVATE
    ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets/")
target_compile_options(RelGalaga PRIVATE -Wall -O2)

# ----- Test executable (no raylib dependency) -----
add_executable(test_relativity
    tests/test_main.cpp
    tests/test_lorentz.cpp
    tests/test_aberration.cpp
    tests/test_doppler.cpp
    src/relativity/BoostFrame.cpp
    src/relativity/Lorentz.cpp
    src/relativity/Aberration.cpp
    src/relativity/Doppler.cpp
)
target_include_directories(test_relativity PRIVATE src)
target_compile_options(test_relativity PRIVATE -Wall -O2)
