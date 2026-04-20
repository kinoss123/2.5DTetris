#ifndef _INPUT_
#define _INPUT_

#include "io.h"

class Game;

enum class GameAction {
    None,
    Quit,
    Left,
    Right,
    Down,
    HardDrop,
    RotateCW,
};

// Input state for held-key repeating.
struct InputState {
    int leftHeldMs = 0;
    int rightHeldMs = 0;
    int leftRepeatMs = 0;
    int rightRepeatMs = 0;

    int downRepeatMs = 0;
};

// Translates IO key codes into gameplay actions (edge + held repeats).
GameAction PollAction(IO &io, int deltaMs, InputState &state);

// Applies a single action to the game.
void ApplyAction(GameAction action, Game &game);

#endif // _INPUT_
