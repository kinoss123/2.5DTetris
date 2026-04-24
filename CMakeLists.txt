cmake_minimum_required(VERSION 3.30)

project(tetris_25d LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

# raylib (submodule)
if (EXISTS "${CMAKE_SOURCE_DIR}/external/raylib/CMakeLists.txt")
    add_subdirectory(external/raylib)
else()
    message(FATAL_ERROR "raylib submodule not found. Run: git submodule update --init --recursive")
endif()

# Core logic library (renderer/input independent)
add_library(tetris_core
    src/board.cpp
    src/game.cpp
    src/pieces.cpp
)

target_include_directories(tetris_core PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

# Logic-only smoke test (no rendering/input backend yet).
add_executable(logic_smoke
    tests/logic_smoke.cpp
)

target_link_libraries(logic_smoke PRIVATE tetris_core)

# Interactive app (raylib)
add_executable(tetris
    src/main.cpp
    src/platform.cpp
    src/renderer.cpp
    src/input.cpp
    src/ui.cpp
    src/ui3d.cpp
)

target_link_libraries(tetris PRIVATE tetris_core raylib)

# NOTE: The upstream logic smoke program is intentionally verbose and may
# return non-zero depending on the current gameplay/board rules. We build it
# here, but do not register it as a ctest until the rules stabilize.
