#include "game.h"

#include <algorithm>
#include <ctime>

namespace
{
// Number of milliseconds that the piece remains before going 1 block down.
static const int kWaitTimeMs = 700;

// When the falling piece is resting on the ground, wait this long before
// locking it (unless hard dropped). This gives the player a small window to
// slide/rotate.
static const int kLockDelayMs = 500;

// Classic line clear pause.
static const int kLineClearPauseMs = 500;

// Score threshold for cube rotation.
static const int kRotateEveryScore = 500;

struct Kick {
    int dx;
    int dy;
};

// SRS wall kicks (clockwise) using this game's coordinate system where +Y is
// down. The published SRS tables use +Y up; we pre-flip dy here.
// Rotation transitions are indexed by the "from" rotation:
//   0->1, 1->2, 2->3, 3->0.
static const Kick kJLSTZ_CW[4][5] = {
    // 0 -> 1: (0,0), (-1,0), (-1,+1), (0,-2), (-1,-2)
    {{0, 0}, {-1, 0}, {-1, -1}, {0, +2}, {-1, +2}},
    // 1 -> 2: (0,0), (+1,0), (+1,-1), (0,+2), (+1,+2)
    {{0, 0}, {+1, 0}, {+1, +1}, {0, -2}, {+1, -2}},
    // 2 -> 3: (0,0), (+1,0), (+1,+1), (0,-2), (+1,-2)
    {{0, 0}, {+1, 0}, {+1, -1}, {0, +2}, {+1, +2}},
    // 3 -> 0: (0,0), (-1,0), (-1,-1), (0,+2), (-1,+2)
    {{0, 0}, {-1, 0}, {-1, +1}, {0, -2}, {-1, -2}},
};

static const Kick kI_CW[4][5] = {
    // 0 -> 1: (0,0), (-2,0), (+1,0), (-2,-1), (+1,+2)
    {{0, 0}, {-2, 0}, {+1, 0}, {-2, +1}, {+1, -2}},
    // 1 -> 2: (0,0), (-1,0), (+2,0), (-1,+2), (+2,-1)
    {{0, 0}, {-1, 0}, {+2, 0}, {-1, -2}, {+2, +1}},
    // 2 -> 3: (0,0), (+2,0), (-1,0), (+2,+1), (-1,-2)
    {{0, 0}, {+2, 0}, {-1, 0}, {+2, -1}, {-1, +2}},
    // 3 -> 0: (0,0), (+1,0), (-2,0), (+1,-2), (-2,+1)
    {{0, 0}, {+1, 0}, {-2, 0}, {+1, +2}, {-2, -1}},
};
} // namespace

Game::Game(Board &board, Pieces &pieces) : mBoard(board), mPieces(pieces)
{
    Reset((std::uint32_t)std::time(nullptr));
}

void Game::RefillBag()
{
    for (int i = 0; i < (int)mBag.size(); i++)
        mBag[(std::size_t)i] = i;
    std::shuffle(mBag.begin(), mBag.end(), mRng);
    mBagPos = 0;
}

int Game::NextPieceFromBag()
{
    if (mBagPos >= (int)mBag.size())
        RefillBag();
    return mBag[(std::size_t)mBagPos++];
}

void Game::Reset(std::uint32_t seed)
{
    mRng.seed(seed);

    // Reset the board state too.
    mBoard.InitBoard();

    mScore = 0;
    mRotateIndex = 0;
    mNextRotateAt = kRotateEveryScore;
    mFallAccumMs = 0;
    mLockAccumMs = 0;
    mPauseMs = 0;
    mPendingSpawn = false;
    mPendingRotateSteps = 0;
    mGameOver = false;
    mHasActivePiece = true;

    // Ensure first NextPieceFromBag() refills deterministically.
    mBagPos = (int)mBag.size();

    // First piece (active)
    mPiece = NextPieceFromBag();
    mRotation = 0;
    mPosX = (BOARD_WIDTH / 2) + mPieces.GetXInitialPosition(mPiece, mRotation);
    mPosY = mPieces.GetYInitialPosition(mPiece, mRotation);

    // Next piece (preview)
    mNextPiece = NextPieceFromBag();
    mNextRotation = 0;
    mNextPosX = BOARD_WIDTH + 5;
    mNextPosY = 5;
}

int Game::ScoreForLines(int lines) const
{
    // Modern flat scoring. If cascades clear >4 lines total, score in chunks.
    int score = 0;
    while (lines >= 4) {
        score += 800;
        lines -= 4;
    }
    switch (lines) {
    case 1:
        return score + 100;
    case 2:
        return score + 300;
    case 3:
        return score + 500;
    default:
        return score;
    }
}

void Game::AdvanceNextRotateAt()
{
    // Rotate every fixed score increment.
    mRotateIndex++;
    mNextRotateAt += kRotateEveryScore;
}

void Game::RollNext()
{
    mNextPiece = NextPieceFromBag();
    mNextRotation = 0;
}

bool Game::CanMoveDown() const
{
    return mBoard.IsPossibleMovement(mPosX, mPosY + 1, mPiece, mRotation);
}

void Game::SpawnFromNext()
{
    mPiece = mNextPiece;
    mRotation = mNextRotation;
    mPosX = (BOARD_WIDTH / 2) + mPieces.GetXInitialPosition(mPiece, mRotation);
    mPosY = mPieces.GetYInitialPosition(mPiece, mRotation);
    RollNext();

    mHasActivePiece = true;

    // Spawn validity check.
    if (!mBoard.IsPossibleMovement(mPosX, mPosY, mPiece, mRotation)) {
        mGameOver = true;
        return;
    }

    mLockAccumMs = 0;
}

void Game::MoveLeft()
{
    if (mGameOver)
        return;
    if (IsPaused())
        return;
    if (mBoard.IsPossibleMovement(mPosX - 1, mPosY, mPiece, mRotation))
        mPosX--;

    if (CanMoveDown())
        mLockAccumMs = 0;
}

void Game::MoveRight()
{
    if (mGameOver)
        return;
    if (IsPaused())
        return;
    if (mBoard.IsPossibleMovement(mPosX + 1, mPosY, mPiece, mRotation))
        mPosX++;

    if (CanMoveDown())
        mLockAccumMs = 0;
}

void Game::SoftDrop()
{
    if (mGameOver)
        return;
    if (IsPaused())
        return;
    if (CanMoveDown()) {
        mPosY++;
        mLockAccumMs = 0;
    } else {
        // Don't lock instantly; let the lock delay apply.
        // If you want "instant lock" on down, change this to LandPiece().
        mLockAccumMs = kLockDelayMs;
    }
}

void Game::HardDrop()
{
    if (mGameOver)
        return;
    if (IsPaused())
        return;

    // Walk down to the first invalid position, then lock at y-1.
    while (mBoard.IsPossibleMovement(mPosX, mPosY, mPiece, mRotation))
        mPosY++;

    // Walk ends on first invalid. Lock at y-1.
    mPosY -= 1;
    LandPiece();
}

void Game::RotateCW()
{
    if (mGameOver)
        return;
    if (IsPaused())
        return;

    const int from = mRotation;
    const int to = (mRotation + 1) % 4;

    // Piece indices: 0 O, 1 I, 2 L, 3 J, 4 Z, 5 S, 6 T
    // O doesn't need kicks; keep it stable.
    const Kick *kicks = nullptr;
    int kickCount = 1;
    Kick onlyInPlace[1] = {{0, 0}};

    if (mPiece == 1) {
        kicks = kI_CW[from];
        kickCount = 5;
    } else if (mPiece == 0) {
        kicks = onlyInPlace;
        kickCount = 1;
    } else {
        kicks = kJLSTZ_CW[from];
        kickCount = 5;
    }

    for (int i = 0; i < kickCount; i++) {
        const int nx = mPosX + kicks[i].dx;
        const int ny = mPosY + kicks[i].dy;
        if (!mBoard.IsPossibleMovement(nx, ny, mPiece, to))
            continue;
        mPosX = nx;
        mPosY = ny;
        mRotation = to;
        break;
    }

    if (CanMoveDown())
        mLockAccumMs = 0;
}

void Game::LandPiece()
{
    // Lock the current falling piece.
    mBoard.StorePiece(mPosX, mPosY, mPiece, mRotation);
    mHasActivePiece = false;

    auto ClearCascades = [&]() -> int {
        int total = 0;
        while (true) {
            int c = mBoard.DeletePossibleLines();
            if (c <= 0)
                break;
            total += c;
        }
        return total;
    };

    // Resolve clears (cascades)
    int clearedTotal = ClearCascades();
    mScore += ScoreForLines(clearedTotal);

    // Queue score-based rotations, but do not apply them yet. We want to pause
    // after landing so the player can see the score/board update.
    while (mScore >= mNextRotateAt) {
        mPendingRotateSteps++;
        AdvanceNextRotateAt();
    }

    if (mBoard.IsGameOver()) {
        mGameOver = true;
        return;
    }

    // Always pause after a landing (classic feel) before applying any pending
    // cube rotation and spawning the next piece.
    mPauseMs = kLineClearPauseMs;
    mPendingSpawn = true;
    return;
}

void Game::Tick(int deltaMs)
{
    if (mGameOver)
        return;
    if (deltaMs <= 0)
        return;

    // Clamp to avoid huge jumps (breakpoints, hitching).
    if (deltaMs > 250)
        deltaMs = 250;

    // Pause (e.g. after line clear): freeze game simulation.
    if (mPauseMs > 0) {
        mPauseMs -= deltaMs;
        if (mPauseMs <= 0) {
            mPauseMs = 0;

            // Apply any pending score rotations after the post-land pause.
            if (mPendingRotateSteps > 0) {
                auto ClearCascades = [&]() -> int {
                    int total = 0;
                    while (true) {
                        int c = mBoard.DeletePossibleLines();
                        if (c <= 0)
                            break;
                        total += c;
                    }
                    return total;
                };

                int clearedTotal = 0;
                while (mPendingRotateSteps > 0) {
                    mBoard.RotateCW();
                    mPendingRotateSteps--;

                    int rotatedClears = ClearCascades();
                    clearedTotal += rotatedClears;
                    mScore += ScoreForLines(rotatedClears);

                    // Rotation clears can raise score enough to queue
                    // additional rotations.
                    while (mScore >= mNextRotateAt) {
                        mPendingRotateSteps++;
                        AdvanceNextRotateAt();
                    }
                }

                if (mBoard.IsGameOver()) {
                    mGameOver = true;
                    return;
                }

                // If rotated clears happened, pause again before spawning.
                if (clearedTotal > 0) {
                    mPauseMs = kLineClearPauseMs;
                    mPendingSpawn = true;
                    return;
                }
            }

            if (mPendingSpawn) {
                mPendingSpawn = false;
                SpawnFromNext();
            }
        }
        return;
    }

    mFallAccumMs += deltaMs;

    // If we're resting, count toward lock.
    if (!CanMoveDown()) {
        mLockAccumMs += deltaMs;
        if (mLockAccumMs >= kLockDelayMs) {
            LandPiece();
            return;
        }
    } else {
        mLockAccumMs = 0;
    }

    while (mFallAccumMs >= kWaitTimeMs && !mGameOver) {
        mFallAccumMs -= kWaitTimeMs;

        if (CanMoveDown()) {
            mPosY++;
        } else {
            // Resting: don't lock immediately; let the lock delay handle it.
            mLockAccumMs = 0;
        }
    }
}
