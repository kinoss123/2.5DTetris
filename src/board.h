#ifndef _BOARD_
#define _BOARD_

#include "pieces.h"

#define BOARD_LINE_WIDTH                                                       \
    6                 // Width of each of the two lines that delimit the board
#define BLOCK_SIZE 16 // Width and Height of each block of a piece
#define BOARD_POSITION                                                         \
    320 // Center position of the board from the left of the screen
#define BOARD_WIDTH 10                     // Board width in blocks
#define BOARD_HEIGHT 20                    // Board height in blocks
#define RING_WIDTH (4 * (BOARD_WIDTH - 1)) // 4 faces sharing edge columns
#define MIN_VERTICAL_MARGIN 20 // Minimum vertical margin for the board limit
#define MIN_HORIZONTAL_MARGIN                                                  \
    20 // Minimum horizontal margin for the board limit
#define PIECE_BLOCKS                                                           \
    5 // Number of horizontal and vertical blocks of a matrix piece

// --------------------------------------------------------------------------------
//									 Board
// --------------------------------------------------------------------------------

class Board {
  public:
    Board(Pieces *pPieces, int pScreenHeight);

    // Clears the whole board/ring.
    void InitBoard();

    int GetXPosInPixels(int pPos) const;
    int GetYPosInPixels(int pPos) const;
    bool IsFreeBlock(int pX, int pY) const;
    bool IsFreeBlock(int face, int x, int y) const;

    // Returns -1 if empty, else 0..6 for the tetromino kind stored in this
    // cell.
    int BlockKind(int pX, int pY) const;
    int BlockKind(int face, int x, int y) const;

    bool IsPossibleMovement(int pX, int pY, int pPiece, int pRotation) const;
    void StorePiece(int pX, int pY, int pPiece, int pRotation);
    // Deletes rows that are complete on any face; deletion applies to all
    // faces.
    int DeletePossibleLines();
    bool IsGameOver() const;
    // Rotate the active face 90 degrees clockwise.
    void RotateCW();

    int ActiveFace() const { return mActiveFace; }

  private:
    // Cell encoding: 0 = empty, 1..7 = tetromino kind + 1.
    int mRing[RING_WIDTH][BOARD_HEIGHT];
    int mActiveFace;
    Pieces *mPieces;
    int mScreenHeight;
    bool mOverflowedTop;

    void DeleteLine(int pY);

    int RingX(int pFace, int pX) const;
    int &At(int pFace, int pX, int pY);
    const int &At(int pFace, int pX, int pY) const;
    int &ActiveAt(int pX, int pY);
    const int &ActiveAt(int pX, int pY) const;
};

#endif // _BOARD_
