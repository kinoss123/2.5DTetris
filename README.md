## 2.5D Tetris

Tetris game in 2.5D

## Installation

### Requirements

- CMake
- MSVC v145 C++ x64/x86 build tools on Windows

### Clone:

git clone --recursive https://github.com/nh-phu/2.5DTetris.git

If you already cloned without --recursive:

git submodule update --init --recursive

### Build

#### Windows:

bat
cmake -S . -B build
cmake --build build --config Release --target tetris
build\tetris.exe

#### Linux:

- GNU make

make

- or CMake

cmake -S . -B build
cmake --build build --target tetris
./build/tetris

## Credits and License

This project is licensed under the [MIT License](LICENSE).

Parts of the core Tetris logic are based on the [Tetris Tutorial](https://javilop.com/gamedev/tetris-tutorial-in-c-platform-independent-focused-in-game-logic-for-beginners/) by Javier López, used under [CC BY 3.0](https://creativecommons.org/licenses/by/3.0/).