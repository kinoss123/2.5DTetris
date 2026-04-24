#ifndef _IO_
#define _IO_

// NOTE: This project previously used SDL/SDL_gfx for rendering and input.
// SDL has been removed. A rendering/input backend (raylib) will be added later.

// ------ Enums -----

// NOTE: Names intentionally avoid collisions with raylib's global color macros
// (RED/GREEN/BLUE/etc).
enum class IOColor : int {
    Black,
    DarkGray,
    LightGray,
    Red,
    Orange,
    Green,
    Blue,
    Cyan,
    Magenta,
    Yellow,
    White,
    Max
};

// Minimal cross-platform key codes for the game loop.
// A future backend (raylib) will map its key values to these.
// NOTE: Names intentionally avoid collisions with raylib's global key macros
// (KEY_LEFT/KEY_RIGHT/etc).
enum class IOKey : int {
    None = -1,
    Escape = 256,
    Left,
    Right,
    Down,
    Z,
    X,
};

// --------------------------------------------------------------------------------
//                                      IO
// --------------------------------------------------------------------------------

class IO {
  public:
    IO();

    ~IO();

    void DrawRectangle(int pX1, int pY1, int pX2, int pY2, IOColor pC);
    void DrawText(int x, int y, const char *text, int fontSize, IOColor color);
    int MeasureTextWidth(const char *text, int fontSize);
    void ClearScreen();
    int GetScreenWidth();
    int GetScreenHeight();
    int InitGraph();
    IOKey Pollkey();
    IOKey Getkey();
    int IsKeyDown(IOKey pKey);
    void UpdateScreen();

    // Raylib wrappers so main doesn't call raylib directly.
    bool ShouldClose();
    int DeltaMs();
};

#endif // _IO_
