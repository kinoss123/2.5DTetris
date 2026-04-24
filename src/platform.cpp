#include "platform.h"

#include <raylib.h>

namespace
{
static const int kWindowWidth = 640;
static const int kWindowHeight = 480;
}

Platform::Platform()
{
    if (!IsWindowReady()) {
        InitWindow(kWindowWidth, kWindowHeight, "2.5D Tetris");
        // We'll handle quit explicitly (ESC mapped in input.cpp).
        SetExitKey(KEY_NULL);
        SetTargetFPS(60);
    }
}

Platform::~Platform()
{
    if (IsWindowReady())
        CloseWindow();
}

bool Platform::ShouldClose() { return ::WindowShouldClose(); }

int Platform::DeltaMs()
{
    int ms = (int)(::GetFrameTime() * 1000.0f);
    if (ms < 0)
        ms = 0;
    // Avoid huge jumps (debugger, hitching).
    if (ms > 250)
        ms = 250;
    return ms;
}
