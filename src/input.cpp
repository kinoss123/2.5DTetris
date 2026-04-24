#include "input.h"

#include "game.h"

#include <raylib.h>

namespace
{
// Delayed auto-shift (DAS) and auto-repeat rate (ARR).
static const int kDASMs = 150;
static const int kARRMs = 50;

// Soft drop repeat while held.
static const int kDownRepeatMs = 33;
} // namespace

GameAction PollAction(int deltaMs, InputState &state)
{
    if (deltaMs < 0)
        deltaMs = 0;
    if (deltaMs > 250)
        deltaMs = 250;

    // Edge-triggered actions via raylib key-press events.
    int key = GetKeyPressed();
    switch (key) {
    case KEY_ESCAPE:
        return GameAction::Quit;
    case KEY_X:
        return GameAction::HardDrop;
    case KEY_Z:
        return GameAction::RotateCW;
    default:
        break;
    }

    bool leftDown = ::IsKeyDown(KEY_LEFT);
    bool rightDown = ::IsKeyDown(KEY_RIGHT);

    // If both are held, cancel horizontal movement.
    if (leftDown && rightDown) {
        leftDown = false;
        rightDown = false;
    }

    // Down is a fast-fall while held (throttled so it's controllable).
    if (::IsKeyDown(KEY_DOWN)) {
        state.downRepeatMs += deltaMs;
        if (state.downRepeatMs >= kDownRepeatMs) {
            state.downRepeatMs = 0;
            return GameAction::Down;
        }
    } else {
        state.downRepeatMs = 0;
    }

    // Left held-repeat.
    if (leftDown) {
        state.leftHeldMs += deltaMs;

        if (state.leftHeldMs == deltaMs) {
            state.leftRepeatMs = 0;
            return GameAction::Left;
        }

        if (state.leftHeldMs >= kDASMs) {
            state.leftRepeatMs += deltaMs;
            while (state.leftRepeatMs >= kARRMs) {
                state.leftRepeatMs -= kARRMs;
                return GameAction::Left;
            }
        }
    } else {
        state.leftHeldMs = 0;
        state.leftRepeatMs = 0;
    }

    // Right held-repeat.
    if (rightDown) {
        state.rightHeldMs += deltaMs;

        if (state.rightHeldMs == deltaMs) {
            state.rightRepeatMs = 0;
            return GameAction::Right;
        }

        if (state.rightHeldMs >= kDASMs) {
            state.rightRepeatMs += deltaMs;
            while (state.rightRepeatMs >= kARRMs) {
                state.rightRepeatMs -= kARRMs;
                return GameAction::Right;
            }
        }
    } else {
        state.rightHeldMs = 0;
        state.rightRepeatMs = 0;
    }

    return GameAction::None;
}

void ApplyAction(GameAction action, Game &game)
{
    switch (action) {
    case GameAction::Left:
        game.MoveLeft();
        break;
    case GameAction::Right:
        game.MoveRight();
        break;
    case GameAction::Down:
        game.SoftDrop();
        break;
    case GameAction::HardDrop:
        game.HardDrop();
        break;
    case GameAction::RotateCW:
        game.RotateCW();
        break;
    case GameAction::Quit:
    case GameAction::None:
        break;
    }
}
