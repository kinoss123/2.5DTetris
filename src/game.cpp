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

// Speed levels: every 5000 points, fall interval drops by this step (ms).
// Minimum interval is capped at kMinFallMs so the game stays playable.

static const int kSpeedUpEveryScore = 200;

static const int kFallStepMs        = 50;  // reduction per level

static const int kMinFallMs         = 100; // fastest possible interval


struct Kick {
    int dx;
    int dy;
};

// SRS wall kicks
// The published SRS tables use +Y up; we pre-flip dy here.
// Rotation transitions are indexed by the "from" rotation:
//   0: spawn state, 1 90° CW, 2 180°, 3 90° CCW
static const Kick JLSTZ_kick[8][5] = {
    // 0 -> 1
    {{0, 0}, {-1, 0}, {-1, -1}, {0, +2}, {-1, +2}},
    // 0 -> 3
    {{0, 0}, {+1, 0}, {+1, -1}, {0, +2}, {+1, +2}},
    // 1 -> 2
    {{0, 0}, {+1, 0}, {+1, +1}, {0, -2}, {+1, -2}},
    // 1 -> 0
    {{0, 0}, {+1, 0}, {+1, +1}, {0, -2}, {+1, -2}},
    // 2 -> 3
    {{0, 0}, {+1, 0}, {+1, -1}, {0, +2}, {+1, +2}},
    // 2 -> 1
    {{0, 0}, {-1, 0}, {-1, -1}, {0, +2}, {-1, +2}},
    // 3 -> 0
    {{0, 0}, {-1, 0}, {-1, +1}, {0, -2}, {-1, -2}},
    // 3 -> 2
    {{0, 0}, {-1, 0}, {-1, +1}, {0, -2}, {-1, -2}},
};

static const Kick I_kick[8][5] = {
    // 0 -> 1
    {{0, 0}, {-2, 0}, {+1, 0}, {-2, +1}, {+1, -2}},
    // 0 -> 3
    {{0, 0}, {-1, 0}, {+2, 0}, {-1, -2}, {+2, +1}},
    // 1 -> 2
    {{0, 0}, {-1, 0}, {+2, 0}, {-1, -2}, {+2, +1}},
    // 1 -> 0
    {{0, 0}, {+2, 0}, {-1, 0}, {+2, -1}, {-1, +2}},
    // 2 -> 3
    {{0, 0}, {+2, 0}, {-1, 0}, {+2, -1}, {-1, +2}},
    // 2 -> 1
    {{0, 0}, {+1, 0}, {-2, 0}, {+1, +2}, {-2, -1}},
    // 3 -> 0
    {{0, 0}, {+1, 0}, {-2, 0}, {+1, +2}, {-2, -1}},
    // 3 -> 2
    {{0, 0}, {-2, 0}, {+1, 0}, {-2, +1}, {+1, -2}},
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

// Returns the fall interval in ms for the current score.

// Starts at kWaitTimeMs (700 ms) and decreases by kFallStepMs every

// kSpeedUpEveryScore points, down to kMinFallMs.

int Game::FallIntervalMs() const

{

    int level    = mScore / kSpeedUpEveryScore; // 0, 1, 2, …

    int interval = kWaitTimeMs - level * kFallStepMs;

    if (interval < kMinFallMs)

        interval = kMinFallMs;

    return interval;

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

void Game::Rotate(int direction)
{
    if (mGameOver)
        return;
    if (IsPaused())
        return;
    if (mPiece == 0) {
        if (CanMoveDown())
            mLockAccumMs = 0;
        return;
    }

    const int state_result = (direction) ? (mRotation - 1 + 4) % 4 : (mRotation + 1) % 4;
    int rotation_index = mRotation * 2 + direction;
    const Kick (*kick_table)[5] = (mPiece == 1) ? I_kick : JLSTZ_kick;

    for (int i = 0; i < 5; i++) {
        const int nx = mPosX + kick_table[rotation_index][i].dx;
        const int ny = mPosY + kick_table[rotation_index][i].dy;

        if (mBoard.IsPossibleMovement(nx, ny, mPiece, state_result)) {
            mPosX = nx;
            mPosY = ny;
            mRotation = state_result;
            break;
        }
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

    while (mFallAccumMs >= FallIntervalMs() && !mGameOver) {
        mFallAccumMs -= FallIntervalMs();

        if (CanMoveDown()) {
            mPosY++;
        } else {
            // Resting: don't lock immediately; let the lock delay handle it.
            mLockAccumMs = 0;
        }
    }
}
