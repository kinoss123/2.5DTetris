#ifndef _GAME_
#define _GAME_

#include "board.h"
#include "pieces.h"

#include <cstdint>

// Game logic + state (no rendering / no input backend).
class Game {
  public:
    Game(Board &board, Pieces &pieces);

    // Resets gameplay state and seeds RNG for deterministic runs.
    void Reset(std::uint32_t seed);

    // Advances the simulation by deltaMs (gravity, lock, clears,
    // score-rotates).
    void Tick(int deltaMs);

    // Player actions.
    void MoveLeft();
    void MoveRight();
    void SoftDrop();
    void HardDrop();
    void RotateCW();

    // State (read-only).
    bool IsGameOver() const { return mGameOver; }
    bool IsPaused() const { return mPauseMs > 0; }
    bool HasActivePiece() const { return mHasActivePiece; }
    int Score() const { return mScore; }

    int ActiveX() const { return mPosX; }
    int ActiveY() const { return mPosY; }
    int ActivePiece() const { return mPiece; }
    int ActiveRotation() const { return mRotation; }

    int NextPiece() const { return mNextPiece; }
    int NextRotation() const { return mNextRotation; }
    int NextX() const { return mNextPosX; }
    int NextY() const { return mNextPosY; }

  private:
    Board &mBoard;
    Pieces &mPieces;

    // Falling piece.
    int mPosX;
    int mPosY;
    int mPiece;
    int mRotation;

    // Next piece (preview).
    int mNextPiece;
    int mNextRotation;
    int mNextPosX;
    int mNextPosY;

    // Scoring + rotation schedule.
    int mScore;
    int mRotateIndex;
    int mNextRotateAt;

    // Gravity.
    int mFallAccumMs;

    // Lock delay when piece is resting on the ground.
    int mLockAccumMs;

    // Pause timer used for line clear delays (classic feel).
    int mPauseMs;
    bool mPendingSpawn;

    // When a landing crosses a rotation threshold, defer applying the
    // rotation until after the post-land pause so the player can see the score.
    int mPendingRotateSteps;

    // True while a controllable falling piece exists. During post-land pauses
    // (and before spawning), we hide the falling piece to avoid double-drawing.
    bool mHasActivePiece;

    bool mGameOver;

    int GetRand(int a, int b);
    void SpawnFromNext();
    void RollNext();

    void LandPiece();
    int ScoreForLines(int lines) const;
    void AdvanceNextRotateAt();

    bool CanMoveDown() const;
};

#endif // _GAME_
