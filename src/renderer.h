#ifndef _RENDERER_
#define _RENDERER_

#include "colors.h"

class Renderer {
  public:
    Renderer();
    ~Renderer();

    void BeginFrame();
    void Clear(RenderColor color);
    void EndFrame();

    void DrawRectangle(int x1, int y1, int x2, int y2, RenderColor color);
    void DrawText(int x, int y, const char *text, int fontSize,
                  RenderColor color);
    int MeasureTextWidth(const char *text, int fontSize);

    int GetScreenWidth();
    int GetScreenHeight();
};

#endif
