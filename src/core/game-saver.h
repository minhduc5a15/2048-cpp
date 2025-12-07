#pragma once
#include <optional>

#include "types.h"

namespace tfe::core {

    class GameSaver {
    public:
        // Saves the game state to a file
        static void save(const GameState& state);

        // Loads the game state (returns std::nullopt if no save file exists)
        static std::optional<GameState> load();

        // Clears the save file (used when the player loses or resets the game)
        static void clearSave();

        // Checks if a save file exists
        static bool hasSave();
    };

}  // namespace tfe::core