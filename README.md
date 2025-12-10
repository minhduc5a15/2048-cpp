# 2048-cpp: High-Performance AI-Ready 2048 Game

A highly optimized C++ implementation of the 2048 game, designed for performance and Reinforcement Learning (RL) research. This project features a blazing fast core engine using **64-bit Bitboards** and **Pre-computed Lookup Tables**, achieving millions of moves per second.

It includes both a classic Console interface (now with an **AI Auto-Play** mode) and a polished **Raylib GUI** version.

## 🚀 Key Features

### ⚡ High-Performance Core
- **Bitboard Representation**: The entire 4x4 board is stored in a single `uint64_t`.
- **Lookup Tables**: Move logic, scoring, and heuristic evaluations are pre-calculated for all $2^{16}$ row states, reducing complex logic to $O(1)$ table lookups.
- **No Memory Allocation**: Critical paths avoid `std::vector` or heap allocations.

### 🧠 Advanced AI & RL Support
- **Expectimax Solver**: A highly optimized C++ Expectimax search agent (Depth 4-6) integrated directly into the game core.
- **Tuple Network Heuristics**: Supports loading learned weights (from Python training) to guide the AI.
- **Python Bindings (`pybind11`)**: Exposes the high-speed C++ core to Python as `py2048`, allowing you to train RL agents in Python while executing the environment in C++.

### 🎮 Multiple Interfaces
- **Console Version**: Fast, lightweight terminal game. Includes **'P'** to toggle AI Auto-Play.
- **GUI Version**: Smooth animations using [Raylib](https://www.raylib.com/).

## 🛠️ Project Structure

```text
├── src/
│   ├── core/           # Bitboard logic, Lookup Tables, AI Solver
│   ├── python-binding/ # Pybind11 wrapper (py2048)
│   ├── game/           # Console Game Loop
│   ├── gui/            # Raylib GUI
│   ├── renderer/       # Console Renderer
│   ├── input/          # Input handling
│   └── utils/          # Utilities
├── ai/                 # Python RL training scripts (Tuple Network)
├── tests/              # GoogleTest unit tests
└── CMakeLists.txt      # Build configuration
```

## 📦 Prerequisites

- **C++ Compiler**: C++20 compliant (GCC, Clang, or MSVC).
- **CMake**: Version 3.15+.
- **Python 3.6+** (for bindings).

## 🔨 How to Build

1. **Clone the repository:**
   ```bash
   git clone https://github.com/minhduc5a15/2048-cpp.git
   cd 2048-cpp
   ```

2. **Compile with CMake:**
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```
   *Note: CMake will automatically fetch dependencies (Raylib, GoogleTest, Pybind11).*

3. **Export AI Weights (Optional):**
   To enable the AI Solver to play smartly, you need to generate the weight file.
   ```bash
   # From project root
   python3 export_weights.py
   # This creates 'build/bin/tuple_weights.bin'
   ```

## 🎮 How to Run

Binaries are located in `build/bin/`.

### Console Game
```bash
./build/bin/2048-game
```
- **WASD / Arrow Keys**: Move.
- **P**: Toggle **AI Auto-Play** (Watch the AI play at high speed!).
- **Q**: Quit.

### GUI Game
```bash
./build/bin/2048-gui
```
- **WASD / Arrow Keys**: Move.

### Python Integration
You can import the C++ core in Python for training:
```python
import py2048

board = py2048.Board()
board.move(py2048.Direction.Up)
print(board.get_grid())
```
*Ensure the generated `py2048.*.so` file is in your Python path.*

## 🧪 Running Tests

To verify the correctness of the Bitboard logic and AI Solver:
```bash
./build/bin/unit_tests
```

## 🧩 Technical Details

- **State Space**: $16$ cells $	imes$ $4$ bits/cell = 64 bits.
- **Movement**: Transposition logic allows reusing "Move Left" tables for all 4 directions.
- **AI Speed**: The Expectimax solver can search thousands of nodes in milliseconds.

## 📜 License

This project is open-source and available for educational and research purposes.