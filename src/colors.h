#ifndef _COLORS_
#define _COLORS_

// Render-facing types shared by UI modules.

// NOTE: Names intentionally avoid collisions with raylib's global color macros
// (RED/GREEN/BLUE/etc).
enum class RenderColor : int {
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

#endif // _COLORS_
