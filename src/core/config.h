#pragma once
#include <cstdint>

/**
 * @file config.h
 * @brief Defines global configuration constants and game rules.
 *
 * This file centralizes all magic numbers and configurable parameters
 * for the game logic, bit manipulation masks, and win conditions.
 */

namespace tfe::core::Config {

    /**
     * @brief The standard size of the game grid (4x4).
     */
    constexpr int DEFAULT_BOARD_SIZE = 4;

    /**
     * @brief The target exponent to reach for a win.
     *
     * 11 corresponds to 2^11 = 2048.
     */
    constexpr int WINNING_EXPONENT = 11;

    /**
     * @brief The probability (0.0 to 1.0) of spawning a '2' tile.
     *
     * The remaining probability (1.0 - SPAWN_PROBABILITY_2) is for spawning a '4'.
     */
    constexpr double SPAWN_PROBABILITY_2 = 0.9;

    /**
     * @brief The exponent value for the lower spawn tile (usually 2^1 = 2).
     */
    constexpr int TILE_EXPONENT_LOW = 1;

    /**
     * @brief The exponent value for the higher spawn tile (usually 2^2 = 4).
     */
    constexpr int TILE_EXPONENT_HIGH = 2;

    /**
     * @brief Bitmask to extract a single row (16 bits) from a 64-bit integer.
     *
     * Used in bitwise operations to isolate row data.
     */
    constexpr uint64_t ROW_MASK = 0xFFFFULL;

    /**
     * @brief Bitmask to extract specific columns.
     *
     * Used primarily for debugging or complex column-wise bit manipulation.
     * Pattern: 0x000F000F000F000F selects the lowest 4 bits (first column) of each row.
     */
    constexpr uint64_t COL_MASK = 0x000F000F000F000FULL;
}  // namespace tfe::core::Config