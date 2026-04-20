# 2.5D Tetris

Tetris game in 2.5D

## Installation

This repo uses raylib as a git submodule.

Clone (recommended):

```bash
git clone --recursive https://github.com/nh-phu/2.5DTetris.git
```

If you already cloned without `--recursive`:

```bash
git submodule update --init --recursive
```

## Build (CMake)

Linux:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target tetris_app
./build/tetris_app
```

Windows:

```bat
git submodule update --init --recursive
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Release --target tetris_app
build\Release\tetris_app.exe
```

## Build (Makefile)

```bash
make
make run
make clean
```

## Credits and License
This project is licensed under the [MIT License](LICENSE).

Parts of the core Tetris logic are based on the [Tetris Tutorial](https://javilop.com/gamedev/tetris-tutorial-in-c-platform-independent-focused-in-game-logic-for-beginners/) by Javier López, used under [CC BY 3.0](https://creativecommons.org/licenses/by/3.0/).
