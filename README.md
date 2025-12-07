# 2048 Game - C++ & Raylib

A modern, modular implementation of the classic 2048 game in C++. This project features a clean architecture that separates core logic from the presentation layer, offering both a **Terminal/Console** version and a graphical **GUI version** powered by Raylib with smooth animations.

## 🚀 Features

-   **Modular Architecture**: Separated into `core`, `game` (logic), `input`, `renderer` (console), and `gui` (raylib) libraries.
-   **Dual Interface**:
    -   **Console**: Uses ANSI escape codes for colors and raw mode for direct input.
    -   **GUI**: Built with [Raylib](https://www.raylib.com/), featuring sliding animations, spawn effects (Zoom), and merge effects (Pop/Pulse).
-   **Modern C++**: Written in C++20 standard.
-   **Unit Testing**: Core logic is tested using [GoogleTest](https://github.com/google/googletest).
-   **Cross-Platform Build**: Managed via CMake with automated dependency fetching.

## 🛠️ Project Structure

```text
├── src/
│   ├── core/       # Game logic (Board, Tile merging, Rules) - No external deps
│   ├── input/      # Input handling (Unix raw mode / Abstract input)
│   ├── renderer/   # Console rendering implementation
│   ├── gui/        # Graphical rendering with Raylib (Animations, Theme)
│   ├── game/       # Main Console Game Loop
│   └── utils/      # Random number generators
├── tests/          # Unit tests for core logic
└── CMakeLists.txt  # Root build configuration