#ifndef _PIECES_
#define _PIECES_

class Pieces {
  public:
    int GetBlockType(int pPiece, int pRotation, int pX, int pY) const;
    int GetXInitialPosition(int pPiece, int pRotation) const;
    int GetYInitialPosition(int pPiece, int pRotation) const;
};

#endif
