#pragma once
#include <vector>

namespace tfe::core {
    // Represents a single tile on the game board, holding its integer value (e.g., 2, 4, 8).
    using Tile = int;

    // Represents the entire game board as a 2D grid of tiles.
    using Grid = std::vector<std::vector<Tile>>;

    // Enum representing the possible directions a player can move the tiles.
    enum class Direction { Up, Down, Left, Right };
}  // namespace tfe::core
