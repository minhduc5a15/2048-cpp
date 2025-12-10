#pragma once
#include <cstdint>
#include <vector>

namespace tfe::core {
    // Tile stores the exponent (0..15). For example: 1 -> 2, 2 -> 4. 0 is an empty tile.
    using Tile = uint8_t;

    // 64-bit Bitboard containing the entire 4x4 board.
    // Memory layout: [Row 3][Row 2][Row 1][Row 0] (Each row is 16 bits)
    using Bitboard = uint64_t;

    // A row of 4 tiles (4x4 bits = 16 bits)
    using Row = uint16_t;

    // The old Grid is kept for GUI usage (rendering actual values to the screen)
    using Grid = std::vector<std::vector<int>>;

    enum class Direction { Up, Down, Left, Right };

    struct GameState {
        Bitboard board;
        int score;
        // Other variables like idGrid are temporarily ignored in the Core AI for speed optimization
    };
}  // namespace tfe::core