#pragma once
#include <optional>

#include "types.h"

namespace tfe::core {

    /**
     * @class GameSaver
     * @brief Handles the persistence of game progress.
     *
     * Provides static methods to save, load, and clear the game state
     * to/from a local file (savegame.json). This allows the player to
     * resume their game after closing the application.
     */
    class GameSaver {
    public:
        /**
         * @brief Saves the current game state to a file.
         * @param state The GameState object to serialize.
         */
        static void save(const GameState& state);

        /**
         * @brief Loads the game state from the save file.
         * @return An optional containing the GameState if successful, or std::nullopt if no save exists.
         */
        static std::optional<GameState> load();

        /**
         * @brief Deletes the save file.
         * 
         * Typically called when the game is manually reset or the player loses,
         * ensuring the next session starts fresh.
         */
        static void clearSave();

        /**
         * @brief Checks if a valid save file exists.
         * @return True if a save file is found, False otherwise.
         */
        static bool hasSave();
    };

}  // namespace tfe::core