#include "board.h"

int Board::RingX(int pFace, int pX) const
{
    int rx = pFace * (BOARD_WIDTH - 1) + pX;
    // Only wrap case is face=3, x=BOARD_WIDTH-1.
    if (rx == RING_WIDTH)
        rx = 0;
    return rx;
}

int &Board::At(int pFace, int pX, int pY)
{
    return mRing[RingX(pFace, pX)][pY];
}

const int &Board::At(int pFace, int pX, int pY) const
{
    return mRing[RingX(pFace, pX)][pY];
}

int &Board::ActiveAt(int pX, int pY) { return At(mActiveFace, pX, pY); }

const int &Board::ActiveAt(int pX, int pY) const
{
    return At(mActiveFace, pX, pY);
}

Board::Board(Pieces *pPieces, int pScreenHeight)
{
    mScreenHeight = pScreenHeight;
    mPieces = pPieces;
    mActiveFace = 0;
    mOverflowedTop = false;

    InitBoard();
}

/*
======================================
Init the board blocks with free positions
======================================
*/
void Board::InitBoard()
{
    for (int x = 0; x < RING_WIDTH; x++)
        for (int y = 0; y < BOARD_HEIGHT; y++)
            mRing[x][y] = 0;
}

/*
======================================
Store a piece in the board by filling the blocks

Parameters:

>> pX:		Horizontal position in blocks
>> pY:		Vertical position in blocks
>> pPiece:	Piece to draw
>> pRotation:	1 of the 4 possible rotations
======================================
*/
void Board::StorePiece(int pX, int pY, int pPiece, int pRotation)
{
    for (int i1 = pX, i2 = 0; i1 < pX + PIECE_BLOCKS; i1++, i2++) {
        for (int j1 = pY, j2 = 0; j1 < pY + PIECE_BLOCKS; j1++, j2++) {
            if (mPieces->GetBlockType(pPiece, pRotation, j2, i2) == 0)
                continue;

            if (j1 < 0) {
                mOverflowedTop = true;
                continue;
            }
            if (i1 < 0 || i1 >= BOARD_WIDTH || j1 >= BOARD_HEIGHT)
                continue;

            // Store the tetromino kind so color stays stable after landing.
            ActiveAt(i1, j1) = pPiece + 1;
        }
    }
}

/*
======================================
Check if the game is over becase a piece have achived the upper position

Returns true or false
======================================
*/
bool Board::IsGameOver() const
{
    if (mOverflowedTop)
        return true;

    // If the first line has blocks in the active face, game over
    for (int x = 0; x < BOARD_WIDTH; x++)
        if (ActiveAt(x, 0) != 0)
            return true;

    return false;
}

/*
======================================
Delete a line of the board by moving all above lines down

Parameters:

>> pY:		Vertical position in blocks of the line to delete
======================================
*/
void Board::DeleteLine(int pY)
{
    // Moves all the upper lines one row down across the whole ring
    for (int y = pY; y > 0; y--)
        for (int rx = 0; rx < RING_WIDTH; rx++)
            mRing[rx][y] = mRing[rx][y - 1];

    // Clear the new top row
    for (int rx = 0; rx < RING_WIDTH; rx++)
        mRing[rx][0] = 0;
}

/*
======================================
Delete all the lines that should be removed
======================================
*/
int Board::DeletePossibleLines()
{
    int cleared = 0;
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        bool anyFaceFull = false;
        for (int face = 0; face < 4 && !anyFaceFull; face++) {
            int x = 0;
            while (x < BOARD_WIDTH) {
                if (At(face, x, y) == 0)
                    break;
                x++;
            }
            if (x == BOARD_WIDTH)
                anyFaceFull = true;
        }

        if (anyFaceFull) {
            DeleteLine(y);
            cleared++;

            y--;
        }
    }

    return cleared;
}

/*
======================================
Returns 1 (true) if the this block of the board is empty, 0 if it is filled

Parameters:

>> pX:		Horizontal position in blocks
>> pY:		Vertical position in blocks
======================================
*/
bool Board::IsFreeBlock(int pX, int pY) const
{
    return (ActiveAt(pX, pY) == 0);
}

bool Board::IsFreeBlock(int face, int x, int y) const
{
    return (At(face, x, y) == 0);
}

int Board::BlockKind(int pX, int pY) const
{
    int v = ActiveAt(pX, pY);
    if (v <= 0)
        return -1;
    return v - 1;
}

int Board::BlockKind(int face, int x, int y) const
{
    int v = At(face, x, y);
    if (v <= 0)
        return -1;
    return v - 1;
}

/*
======================================
Returns the horizontal position (isn pixels) of the block given like parameter

Parameters:

>> pPos:	Horizontal position of the block in the board
======================================
*/
int Board::GetXPosInPixels(int pPos) const
{
    return ((BOARD_POSITION - (BLOCK_SIZE * (BOARD_WIDTH / 2))) +
            (pPos * BLOCK_SIZE));
}

/*
======================================
Returns the vertical position (in pixels) of the block given like parameter

Parameters:

>> pPos:	Horizontal position of the block in the board
======================================
*/
int Board::GetYPosInPixels(int pPos) const
{
    return ((mScreenHeight - (BLOCK_SIZE * BOARD_HEIGHT)) +
            (pPos * BLOCK_SIZE));
}

/*
======================================
Check if the piece can be stored at this position without any collision
Returns true if the movement is  possible, false if it not possible

Parameters:

>> pX:		Horizontal position in blocks
>> pY:		Vertical position in blocks
>> pPiece:	Piece to draw
>> pRotation:	1 of the 4 possible rotations
======================================
*/
bool Board::IsPossibleMovement(int pX, int pY, int pPiece, int pRotation) const
{
    for (int i1 = pX, i2 = 0; i1 < pX + PIECE_BLOCKS; i1++, i2++) {
        for (int j1 = pY, j2 = 0; j1 < pY + PIECE_BLOCKS; j1++, j2++) {
            // Check the limits of the board
            if (i1 < 0 || i1 > BOARD_WIDTH - 1 || j1 > BOARD_HEIGHT - 1) {
                if (mPieces->GetBlockType(pPiece, pRotation, j2, i2) != 0)
                    return 0;
            }

            // Check collision with a block placed in the map
            if (j1 >= 0) {
                if ((mPieces->GetBlockType(pPiece, pRotation, j2, i2) != 0) &&
                    (!IsFreeBlock(i1, j1)))
                    return false;
            }
        }
    }

    return true;
}

/*
=====================================
Rotate the active face 90deg
 * clockwis

This just changes which face window we consider active; the
 * shared-edge rin
storage keeps corner columns globally
 * consistent
=====================================
*/
void Board::RotateCW() { mActiveFace = (mActiveFace + 1) % 4; }
