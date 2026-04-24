#ifndef _UI_
#define _UI_

#include "board.h"
#include "game.h"
#include "renderer.h"
#include "pieces.h"

class UI {
  public:
    static void Draw(Renderer &r, const Board &board, const Pieces &pieces,
                     const Game &game, int screenHeight);

    static void DrawHud(Renderer &r, const Board &board, const Pieces &pieces,
                        const Game &game);

  private:
    static void DrawBoard(Renderer &r, const Board &board, int screenHeight);
    static void DrawPiece(Renderer &r, const Board &board, const Pieces &pieces,
                          int x, int y, int piece, int rotation);
};

#endif
