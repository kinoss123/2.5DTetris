#include "io.h"

#include <raylib.h>

#include <cstdio>

namespace
{
static const int kWindowWidth = 640;
static const int kWindowHeight = 480;

static Font gHudFont;
static bool gHudFontLoaded = false;

static bool FileExists(const char *path)
{
    if (!path)
        return false;
    std::FILE *f = std::fopen(path, "rb");
    if (!f)
        return false;
    std::fclose(f);
    return true;
}

static void LoadHudFontOnce()
{
    if (gHudFontLoaded)
        return;

    // Prefer a mono font. We intentionally don't hard-require a bundled font
    // asset yet; this tries common system paths and falls back to raylib's
    // default.
    const char *candidates[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationMono-Regular.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
        "/System/Library/Fonts/Menlo.ttc",
        "/Library/Fonts/Menlo.ttc",
    };

    for (const char *path : candidates) {
        if (!FileExists(path))
            continue;
        gHudFont = LoadFontEx(path, 64, nullptr, 0);
        if (gHudFont.texture.id != 0) {
            SetTextureFilter(gHudFont.texture, TEXTURE_FILTER_BILINEAR);
            gHudFontLoaded = true;
            break;
        }
    }
}

static void UnloadHudFontOnce()
{
    if (!gHudFontLoaded)
        return;
    UnloadFont(gHudFont);
    gHudFontLoaded = false;
}

static Font HudFont() { return gHudFontLoaded ? gHudFont : GetFontDefault(); }

static Color ToRayColor(IOColor c)
{
    switch (c) {
    case IOColor::Black:
        return BLACK;
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
    UnloadHudFontOnce();
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

        LoadHudFontOnce();
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

    // Prefer nicer TTF font when available.
    Font f = HudFont();
    if (f.texture.id != 0 && gHudFontLoaded) {
        ::DrawTextEx(f, text, {(float)x, (float)y}, (float)fontSize, 1.0f,
                     ToRayColor(color));
        return;
    }

    ::DrawText(text, x, y, fontSize, ToRayColor(color));
}

int IO::MeasureTextWidth(const char *text, int fontSize)
{
    if (!text)
        return 0;

    Font f = HudFont();
    if (f.texture.id != 0 && gHudFontLoaded) {
        Vector2 sz = ::MeasureTextEx(f, text, (float)fontSize, 1.0f);
        return (int)sz.x;
    }

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
