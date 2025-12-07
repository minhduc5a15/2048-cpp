// src/core/config.h
#pragma once

namespace tfe::core::Config {
    // Game Rules
    constexpr int DEFAULT_BOARD_SIZE = 4;
    constexpr int WINNING_TILE = 2048;

    // Spawning Probabilities
    constexpr double SPAWN_PROBABILITY_2 = 0.9;
    constexpr int TILE_VALUE_LOW = 2;
    constexpr int TILE_VALUE_HIGH = 4;
}