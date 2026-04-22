#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <algorithm>
#include <array>
#include <random>

// Test-only access to board internals for printing.
#define private public
#include "board.h"
#undef private

#include "pieces.h"

static void PrintFace(Board &b, int face)
{
    int old = b.mActiveFace;
    b.mActiveFace = face;

    std::printf("face=%d\n", face);
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++)
            std::printf("%c", b.IsFreeBlock(x, y) ? '.' : '#');
        std::printf("\n");
    }

    b.mActiveFace = old;
}

static void PrintAllFaces(Board &b)
{
    for (int f = 0; f < 4; f++)
        PrintFace(b, f);
}

static void AssertSharedEdges(Board &b)
{
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        // face0 right == face1 left
        if (b.At(0, BOARD_WIDTH - 1, y) != b.At(1, 0, y)) {
            std::printf("FAIL: shared edge mismatch f0r!=f1l at y=%d\n", y);
            std::exit(1);
        }
        // face1 right == face2 left
        if (b.At(1, BOARD_WIDTH - 1, y) != b.At(2, 0, y)) {
            std::printf("FAIL: shared edge mismatch f1r!=f2l at y=%d\n", y);
            std::exit(1);
        }
        // face2 right == face3 left
        if (b.At(2, BOARD_WIDTH - 1, y) != b.At(3, 0, y)) {
            std::printf("FAIL: shared edge mismatch f2r!=f3l at y=%d\n", y);
            std::exit(1);
        }
        // face3 right == face0 left (wrap)
        if (b.At(3, BOARD_WIDTH - 1, y) != b.At(0, 0, y)) {
            std::printf("FAIL: shared edge mismatch f3r!=f0l at y=%d\n", y);
            std::exit(1);
        }
    }
}

static int ScoreCount(int lines)
{
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

static int NextRotateAt(int rotateIndex)
{
    // Must match Game's rotation schedule.
    return (rotateIndex + 1) * 500;
}

int main(int argc, char **argv)
{
    unsigned int seed = (argc >= 2)
                            ? (unsigned int)std::strtoul(argv[1], nullptr, 10)
                            : (unsigned int)std::time(nullptr);
    int pieceCount = (argc >= 3) ? std::atoi(argv[2]) : 50;

    std::printf("seed=%u pieces=%d\n", seed, pieceCount);

    std::mt19937 rng(seed);
    std::array<int, 7> bag;
    int bagPos = (int)bag.size();

    auto NextPieceFromBag = [&]() -> int {
        if (bagPos >= (int)bag.size()) {
            for (int i = 0; i < (int)bag.size(); i++)
                bag[(std::size_t)i] = i;
            std::shuffle(bag.begin(), bag.end(), rng);
            bagPos = 0;
        }
        return bag[(std::size_t)bagPos++];
    };

    Pieces pieces;
    Board board(&pieces, /*screenHeight*/ 480);

    int score = 0;
    int rotateIndex = 0;
    int landed = 0;

    for (int n = 0; n < pieceCount; n++) {
        int piece = NextPieceFromBag();
        int rot = 0;
        int y = pieces.GetYInitialPosition(piece, rot);

        // Edge-focused spawn: push the piece as far right as possible (walls
        // enabled). We intentionally start out-of-bounds and walk left to the
        // first valid x.
        int x = BOARD_WIDTH;
        while (x > -PIECE_BLOCKS && !board.IsPossibleMovement(x, y, piece, rot))
            x--;

        std::printf("\nspawn n=%d face=%d piece=%d rot=%d x=%d y=%d score=%d\n",
                    n, board.mActiveFace, piece, rot, x, y, score);

        if (!board.IsPossibleMovement(x, y, piece, rot)) {
            std::printf("GAME OVER: cannot spawn\n");
            PrintAllFaces(board);
            return 1;
        }

        while (board.IsPossibleMovement(x, y + 1, piece, rot))
            y++;
        board.StorePiece(x, y, piece, rot);
        landed++;

        // Resolve clears (cascades)
        int clearedTotal = 0;
        while (true) {
            int c = board.DeletePossibleLines();
            if (c <= 0)
                break;
            clearedTotal += c;
        }
        score += ScoreCount(clearedTotal);

        // Rotate after 5 landed pieces (smoke behavior)
        if (landed % 5 == 0) {
            std::printf("rotate_every_5: rotating once\n");
            board.RotateCW();
            while (board.DeletePossibleLines() > 0) {
            }
        }

        // Rotate on score thresholds (the real rule)
        while (score >= NextRotateAt(rotateIndex)) {
            std::printf("score_rotate: score=%d >= %d, rotating\n", score,
                        NextRotateAt(rotateIndex));
            board.RotateCW();
            rotateIndex++;
            while (board.DeletePossibleLines() > 0) {
            }
        }

        if (board.IsGameOver()) {
            std::printf("GAME OVER: overflowed\n");
            PrintAllFaces(board);
            return 1;
        }

        AssertSharedEdges(board);
        PrintAllFaces(board);
    }

    std::printf("\nPASS: completed %d pieces, score=%d\n", pieceCount, score);
    return 0;
}
