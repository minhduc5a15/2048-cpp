# 2048 Game - C++ & Raylib

A modern, modular implementation of the classic 2048 game in C++. This project features a clean architecture that separates core logic from the presentation layer, offering both a **Terminal/Console** version and a graphical **GUI version** powered by Raylib with smooth animations.

## 🚀 Features

- **Modular Architecture**: Separated into `core`, `game` (logic), `input`, `renderer` (console), and `gui` (raylib) libraries.
- **Dual Interface**:
  - **Console**: Uses ANSI escape codes for colors and raw mode for direct input.
  - **GUI**: Built with [Raylib](https://www.raylib.com/), featuring sliding animations, spawn effects (Zoom), and merge effects (Pop/Pulse).
- **Modern C++**: Written in C++20 standard.
- **Unit Testing**: Core logic is tested using [GoogleTest](https://github.com/google/googletest).
- **Cross-Platform Build**: Managed via CMake with automated dependency fetching.

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
```

## 📦 Prerequisites

- **C++ Compiler**: Support for C++20 (GCC, Clang, or MSVC).
- **CMake**: Version 3.15 or higher.
- **Internet Connection**: Required for CMake to fetch Raylib and GoogleTest automatically.

## 🔨 How to Build

1. Clone the repository:

    ```bash
    git clone [https://github.com/minhduc5a15/2048-cpp.git](https://github.com/minhduc5a15/2048-cpp.git)
    cd 2048-cpp
    ```

2. Create a build directory and compile:

    ```bash
    mkdir build
    cd build
    cmake ..
    cmake --build .
    ```

## 🎮 How to Run

After building, the executables will be located in the `bin/` folder inside your build directory.

### GUI Version (Recommended)

Enjoy smooth animations and colors.

```bash
./bin/2048-gui
```

_Controls:_ Use **Arrow Keys** or **WASD** to move.

### Console Version

Classic terminal experience.

```bash
./bin/2048-game
```

_Controls:_ **WASD** to move, **Q** to quit.

### Running Tests

Verify the core logic integrity.

```bash
./bin/unit_tests
```

## 🧩 Technical Details

- **Animations**: The GUI uses a state-based animation system. Moving tiles interpolate positions linearly, while spawning and merging tiles use `EaseOutBack` and `EasePop` easing functions for a polished feel.
- **Input Handling**: The console version modifies `termios` to enable raw mode, allowing key presses to be detected immediately without hitting Enter.

## 📜 License

This project is open-source and available for educational purposes
