#pragma once
#include <vector>

namespace tfe::core {

    using Tile = int;
    using Grid = std::vector<std::vector<Tile>>;

    enum class Direction {
        Up,
        Down,
        Left,
        Right
    };

} // namespace tfe::core