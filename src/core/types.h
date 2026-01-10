#pragma once
#include <cstdint>
#include <vector>

/**
 * @file types.h
 * @brief Defines the fundamental data types and structures used throughout the core engine.
 *
 * This file contains the type aliases and structures that represent the game state,
 * including the efficient bitboard representation used for the AI solver and the
 * standard grid format for the GUI.
 */

namespace tfe::core {

    /**
     * @brief Represents a single tile on the board.
     *
     * Stores the exponent of the power of 2 (0..15).
     * Value 0 represents an empty tile.
     * Value 1 represents 2^1 = 2.
     * Value 11 represents 2^11 = 2048.
     */
    using Tile = uint8_t;

    /**
     * @brief Represents the entire 4x4 board state in a single 64-bit integer.
     *
     * This is a critical optimization for the AI solver.
     * Memory layout: [Row 3][Row 2][Row 1][Row 0]
     * Each row occupies 16 bits (4 tiles * 4 bits/tile).
     */
    using Bitboard = uint64_t;

    /**
     * @brief Represents a single row of 4 tiles (16 bits).
     *
     * Used for lookup table indices and row-wise operations.
     */
    using Row = uint16_t;

    /**
     * @brief Represents the board as a 2D vector of integer values.
     *
     * This format is less efficient than Bitboard but easier to use for
     * GUI rendering and debugging. It stores actual values (0, 2, 4, 8...)
     * rather than exponents.
     */
    using Grid = std::vector<std::vector<int>>;

    /**
     * @brief Enumeration of possible move directions.
     */
    enum class Direction { Up, Down, Left, Right };

    /**
     * @brief Encapsulates the complete state of the game at a specific moment.
     *
     * Used for saving/loading the game and for AI state evaluation.
     */
    struct GameState {
        Bitboard board; // The current configuration of tiles
        int score;      // The current score
        // Additional state variables can be added here as needed (e.g., turn count)
    };
}  // namespace tfe::core