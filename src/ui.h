#ifndef _UI_
#define _UI_

#include "board.h"
#include "game.h"
#include "io.h"
#include "pieces.h"

// Rendering-only helpers.
class UI {
  public:
    static void Draw(IO &io, const Board &board, const Pieces &pieces,
                     const Game &game, int screenHeight);

    static void DrawHud(IO &io, const Board &board, const Pieces &pieces,
                        const Game &game);

  private:
    static void DrawBoard(IO &io, const Board &board, int screenHeight);
    static void DrawPiece(IO &io, const Board &board, const Pieces &pieces,
                          int x, int y, int piece, int rotation);
};

#endif // _UI_
