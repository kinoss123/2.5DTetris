#include "renderer.h"

#include <raylib.h>

#include <cstdio>
#include <string>

namespace
{
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

    static const char *kRelFontPath = "data/ttf/PressStart2P-Regular.ttf";

    std::string appDir = GetApplicationDirectory();
    if (!appDir.empty() && appDir.back() != '/' && appDir.back() != '\\')
        appDir.push_back('/');

    std::string cand1 = appDir + kRelFontPath;
    std::string cand2 = appDir + "../" + kRelFontPath;
    const char *cand3 = kRelFontPath;

    const char *chosen = nullptr;
    if (FileExists(cand1.c_str()))
        chosen = cand1.c_str();
    else if (FileExists(cand2.c_str()))
        chosen = cand2.c_str();
    else if (FileExists(cand3))
        chosen = cand3;

    if (!chosen)
        return;

    gHudFont = LoadFontEx(chosen, 32, nullptr, 0);
    if (gHudFont.texture.id == 0)
        return;

    SetTextureFilter(gHudFont.texture, TEXTURE_FILTER_POINT);
    gHudFontLoaded = true;
}

static void UnloadHudFontOnce()
{
    if (!gHudFontLoaded)
        return;
    UnloadFont(gHudFont);
    gHudFontLoaded = false;
}

static Font HudFont() { return gHudFontLoaded ? gHudFont : GetFontDefault(); }

static float HudFontSpacing() { return 0.0f; }

static Color ToRayColor(RenderColor c)
{
    switch (c) {
    case RenderColor::Black:
        return BLACK;
    case RenderColor::DarkGray:
        return {26, 26, 30, 255};
    case RenderColor::LightGray:
        return {200, 200, 210, 255};
    case RenderColor::Red:
        return RED;
    case RenderColor::Orange:
        return ORANGE;
    case RenderColor::Green:
        return GREEN;
    case RenderColor::Blue:
        return BLUE;
    case RenderColor::Cyan:
        return SKYBLUE;
    case RenderColor::Magenta:
        return MAGENTA;
    case RenderColor::Yellow:
        return YELLOW;
    case RenderColor::White:
        return RAYWHITE;
    case RenderColor::Max:
    default:
        return RAYWHITE;
    }
}
}

Renderer::Renderer() { LoadHudFontOnce(); }

Renderer::~Renderer() { UnloadHudFontOnce(); }

void Renderer::BeginFrame() { BeginDrawing(); }

void Renderer::Clear(RenderColor color) { ClearBackground(ToRayColor(color)); }

void Renderer::EndFrame() { EndDrawing(); }

void Renderer::DrawRectangle(int x1, int y1, int x2, int y2, RenderColor color)
{
    int w = (x2 - x1) + 1;
    int h = (y2 - y1) + 1;
    if (w <= 0 || h <= 0)
        return;
    ::DrawRectangle(x1, y1, w, h, ToRayColor(color));
}

void Renderer::DrawText(int x, int y, const char *text, int fontSize,
                        RenderColor color)
{
    if (!text)
        return;

    Font f = HudFont();
    ::DrawTextEx(f, text, {(float)x, (float)y}, (float)fontSize,
                 HudFontSpacing(), ToRayColor(color));
}

int Renderer::MeasureTextWidth(const char *text, int fontSize)
{
    if (!text)
        return 0;
    Font f = HudFont();
    Vector2 sz = ::MeasureTextEx(f, text, (float)fontSize, HudFontSpacing());
    return (int)sz.x;
}

int Renderer::GetScreenWidth() { return ::GetScreenWidth(); }

int Renderer::GetScreenHeight() { return ::GetScreenHeight(); }
