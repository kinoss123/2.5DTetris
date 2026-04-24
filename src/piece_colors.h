#ifndef _PIECE_COLORS_
#define _PIECE_COLORS_

#include "colors.h"

struct RGBA8 {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

enum class PieceColorId : unsigned char {
    Yellow,
    Cyan,
    Orange,
    Blue,
    Red,
    Green,
    Purple,
    White,
};

inline PieceColorId PieceColorIdForKind(int kind)
{
    // 0 O, 1 I, 2 L, 3 J, 4 Z, 5 S, 6 T
    switch (kind) {
    case 0:
        return PieceColorId::Yellow;
    case 1:
        return PieceColorId::Cyan;
    case 2:
        return PieceColorId::Orange;
    case 3:
        return PieceColorId::Blue;
    case 4:
        return PieceColorId::Red;
    case 5:
        return PieceColorId::Green;
    case 6:
        return PieceColorId::Purple;
    default:
        return PieceColorId::White;
    }
}

inline RenderColor PieceRenderColorForKind(int kind)
{
    switch (PieceColorIdForKind(kind)) {
    case PieceColorId::Yellow:
        return RenderColor::Yellow;
    case PieceColorId::Cyan:
        return RenderColor::Cyan;
    case PieceColorId::Orange:
        return RenderColor::Orange;
    case PieceColorId::Blue:
        return RenderColor::Blue;
    case PieceColorId::Red:
        return RenderColor::Red;
    case PieceColorId::Green:
        return RenderColor::Green;
    case PieceColorId::Purple:
        return RenderColor::Magenta;
    case PieceColorId::White:
    default:
        return RenderColor::White;
    }
}

inline RGBA8 PieceRGBAForKind(int kind)
{
    switch (PieceColorIdForKind(kind)) {
    case PieceColorId::Yellow:
        return {253, 249, 0, 255};
    case PieceColorId::Cyan:
        return {102, 191, 255, 255};
    case PieceColorId::Orange:
        return {255, 161, 0, 255};
    case PieceColorId::Blue:
        return {0, 121, 241, 255};
    case PieceColorId::Red:
        return {230, 41, 55, 255};
    case PieceColorId::Green:
        return {0, 228, 48, 255};
    case PieceColorId::Purple:
        return {200, 122, 255, 255};
    case PieceColorId::White:
    default:
        return {245, 245, 245, 255};
    }
}

#endif
