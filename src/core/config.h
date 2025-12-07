// src/core/config.h
#pragma once

namespace tfe::core::Config {
    // Defines the rules and parameters of the game.

    // The default size of the game board (width and height).
    constexpr int DEFAULT_BOARD_SIZE = 4;

    // The tile value that the player must reach to win the game.
    constexpr int WINNING_TILE = 2048;


    // Defines the probabilities for new tiles spawning on the board.

    // The probability (from 0.0 to 1.0) that a new tile will have the lower value.
    constexpr double SPAWN_PROBABILITY_2 = 0.9;

    // The lower value for a new tile (typically 2).
    constexpr int TILE_VALUE_LOW = 2;

    // The higher value for a new tile (typically 4).
    constexpr int TILE_VALUE_HIGH = 4;
}