#include "io.h"

#include <raylib.h>

namespace
{
static const int kWindowWidth = 640;
static const int kWindowHeight = 480;

static Color ToRayColor(IOColor c)
{
    switch (c) {
    case IOColor::Black:
        return BLACK;
    case IOColor::DarkGray:
        return {26, 26, 30, 255};
    case IOColor::LightGray:
        return {200, 200, 210, 255};
    case IOColor::Red:
        return RED;
    case IOColor::Orange:
        return ORANGE;
    case IOColor::Green:
        return GREEN;
    case IOColor::Blue:
        return BLUE;
    case IOColor::Cyan:
        return SKYBLUE;
    case IOColor::Magenta:
        return MAGENTA;
    case IOColor::Yellow:
        return YELLOW;
    case IOColor::White:
        return RAYWHITE;
    case IOColor::Max:
    default:
        return RAYWHITE;
    }
}

static IOKey ToIOKey(int key)
{
    switch (key) {
    case KEY_ESCAPE:
        return IOKey::Escape;
    case KEY_LEFT:
        return IOKey::Left;
    case KEY_RIGHT:
        return IOKey::Right;
    case KEY_DOWN:
        return IOKey::Down;
    case KEY_Z:
        return IOKey::Z;
    case KEY_X:
        return IOKey::X;
    default:
        return IOKey::None;
    }
}

static int ToRayKey(IOKey key)
{
    switch (key) {
    case IOKey::Escape:
        return KEY_ESCAPE;
    case IOKey::Left:
        return KEY_LEFT;
    case IOKey::Right:
        return KEY_RIGHT;
    case IOKey::Down:
        return KEY_DOWN;
    case IOKey::Z:
        return KEY_Z;
    case IOKey::X:
        return KEY_X;
    case IOKey::None:
    default:
        return 0;
    }
}
} // namespace

IO::IO() { InitGraph(); }

IO::~IO()
{
    if (IsWindowReady())
        CloseWindow();
}

int IO::InitGraph()
{
    if (!IsWindowReady()) {
        InitWindow(kWindowWidth, kWindowHeight, "2.5D Tetris");
        // We'll handle quit explicitly (ESC mapped in input.cpp).
        SetExitKey(KEY_NULL);
        SetTargetFPS(60);

        // Crisp pixel HUD text.
        SetTextureFilter(GetFontDefault().texture, TEXTURE_FILTER_POINT);
    }
    return 0;
}

void IO::ClearScreen()
{
    BeginDrawing();
    ClearBackground(BLACK);
}

void IO::DrawRectangle(int pX1, int pY1, int pX2, int pY2, IOColor pC)
{
    int w = (pX2 - pX1) + 1;
    int h = (pY2 - pY1) + 1;
    if (w <= 0 || h <= 0)
        return;
    ::DrawRectangle(pX1, pY1, w, h, ToRayColor(pC));
}

void IO::DrawText(int x, int y, const char *text, int fontSize, IOColor color)
{
    if (!text)
        return;

    ::DrawText(text, x, y, fontSize, ToRayColor(color));
}

int IO::MeasureTextWidth(const char *text, int fontSize)
{
    if (!text)
        return 0;

    return ::MeasureText(text, fontSize);
}

int IO::GetScreenWidth() { return ::GetScreenWidth(); }

int IO::GetScreenHeight() { return ::GetScreenHeight(); }

void IO::UpdateScreen() { EndDrawing(); }

bool IO::ShouldClose() { return ::WindowShouldClose(); }

int IO::DeltaMs()
{
    int ms = (int)(::GetFrameTime() * 1000.0f);
    if (ms < 0)
        ms = 0;
    // Avoid huge jumps (debugger, hitching).
    if (ms > 250)
        ms = 250;
    return ms;
}

IOKey IO::Pollkey()
{
    int k = GetKeyPressed();
    return ToIOKey(k);
}

IOKey IO::Getkey() { return Pollkey(); }

int IO::IsKeyDown(IOKey pKey)
{
    int rk = ToRayKey(pKey);
    if (rk == 0)
        return 0;
    return ::IsKeyDown(rk) ? 1 : 0;
}
