#pragma once
#include <cstdint>

namespace tfe::core::Config {
    // Defines the rules and parameters of the game.

    constexpr int DEFAULT_BOARD_SIZE = 4;

    // 2^11 = 2048. AI needs to reach exponent 11.
    constexpr int WINNING_EXPONENT = 11;

    constexpr double SPAWN_PROBABILITY_2 = 0.9;

    // Exponent value when spawned
    constexpr int TILE_EXPONENT_LOW = 1;   // 2^1 = 2
    constexpr int TILE_EXPONENT_HIGH = 2;  // 2^2 = 4

    // Bitmask to get the last 16 bits (representing a row)
    constexpr uint64_t ROW_MASK = 0xFFFFULL;

    // Bitmask to get columns (used for debugging or complex column operations)
    constexpr uint64_t COL_MASK = 0x000F000F000F000FULL;
}  // namespace tfe::core::Config