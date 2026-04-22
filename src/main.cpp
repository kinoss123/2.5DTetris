#include "game.h"
#include "input.h"
#include "ui.h"
#include "ui3d.h"

#include <cstring>

namespace
{
static float EaseInOut(float t)
{
    if (t <= 0.0f)
        return 0.0f;
    if (t >= 1.0f)
        return 1.0f;
    return t * t * (3.0f - 2.0f * t);
}
}

int main(int argc, char **argv)
{
    bool rotateDemo = false;

    // Very small arg parser.
    // Usage: ./tetris_app --rotate-demo
    for (int i = 1; i < argc; i++) {
        if (argv[i] && std::strcmp(argv[i], "--rotate-demo") == 0)
            rotateDemo = true;
    }

    IO mIO;
    int mScreenHeight = mIO.GetScreenHeight();

    // Pieces
    Pieces mPieces;

    // Board
    Board mBoard(&mPieces, mScreenHeight);

    // Game (logic-only)
    Game mGame(mBoard, mPieces);

    InputState inputState;

    struct {
        bool active = false;
        int elapsedMs = 0;
        int durationMs = 500;
        int queuedSteps = 0;
        int stepFromFace = 0;
    } anim;

    // If we're in rotate demo mode, pre-fill each face with a couple pieces so
    // the 3D rotation is visually obvious.
    if (rotateDemo) {
        for (int f = 0; f < 4; f++) {
            // Active face is f after these rotations.
            while (mBoard.ActiveFace() != f)
                mBoard.RotateCW();
            // Place a couple safe pieces near the bottom.
            mBoard.StorePiece(3, BOARD_HEIGHT - 4, 0 /*O*/, 0);
            mBoard.StorePiece(6, BOARD_HEIGHT - 6, 1 /*I*/, 1);
        }
        while (mBoard.ActiveFace() != 0)
            mBoard.RotateCW();
    }

    int rotateDemoAccumMs = 0;

    // ----- Main Loop -----
    while (true) {
        if (mIO.ShouldClose())
            break;

        int deltaMs = mIO.DeltaMs();

        // In rotate demo mode, rotate every so often when idle.
        if (rotateDemo && !anim.active) {
            rotateDemoAccumMs += deltaMs;
            if (rotateDemoAccumMs >= 800) {
                rotateDemoAccumMs = 0;
                int prevFace = mBoard.ActiveFace();
                mBoard.RotateCW();
                int currentFace = mBoard.ActiveFace();
                int steps = (currentFace - prevFace + 4) % 4;
                if (steps > 0) {
                    anim.active = true;
                    anim.elapsedMs = 0;
                    anim.queuedSteps = steps;
                    anim.stepFromFace = prevFace;
                }
            }
        }

        if (anim.active) {
            // ----- Draw -----
            mIO.ClearScreen();

            // Animate a 90deg turn per queued step.
            float t = (float)anim.elapsedMs / (float)anim.durationMs;
            float e = EaseInOut(t);
            float angle = e * 90.0f;
            UI3D::DrawRotate(mBoard, anim.stepFromFace, angle);
            UI::DrawHud(mIO, mBoard, mPieces, mGame);

            mIO.UpdateScreen();

            // Freeze everything during the rotate animation.
            anim.elapsedMs += deltaMs;
            if (anim.elapsedMs >= anim.durationMs) {
                anim.elapsedMs = 0;
                anim.queuedSteps--;
                anim.stepFromFace = (anim.stepFromFace + 1) % 4;
                if (anim.queuedSteps <= 0) {
                    anim.active = false;
                }
            }
        } else {
            // ----- Input -----
            GameAction action = PollAction(mIO, deltaMs, inputState);
            if (action == GameAction::Quit)
                break;
            if (!mGame.IsPaused())
                ApplyAction(action, mGame);

            // ----- Simulation -----
            int prevFace = mBoard.ActiveFace();
            mGame.Tick(deltaMs);
            if (mGame.IsGameOver())
                break;

            // Start rotate animation if the logic rotated the cube this tick.
            int currentFace = mBoard.ActiveFace();
            int steps = (currentFace - prevFace + 4) % 4;
            if (steps > 0) {
                anim.active = true;
                anim.elapsedMs = 0;
                anim.queuedSteps = steps;
                anim.stepFromFace = prevFace;
            }

            // ----- Draw -----
            mIO.ClearScreen();
            if (anim.active) {
                // Avoid a 1-frame "blink" to the new face before the rotation
                // animation starts.
                UI3D::DrawRotate(mBoard, anim.stepFromFace, 0.0f);
            } else {
                UI3D::DrawActiveFace(mBoard, mPieces, mGame);
            }
            UI::DrawHud(mIO, mBoard, mPieces, mGame);
            mIO.UpdateScreen();
        }
    }

    UI3D::Shutdown();

    return 0;
}
