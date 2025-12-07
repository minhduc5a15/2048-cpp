#pragma once

#include <string>

namespace tfe::score {

    /**
     * @class ScoreManager
     * @brief A static utility class to manage loading and saving game scores.
     *
     * This class handles all file I/O for game results, storing them in a JSON file.
     */
    class ScoreManager {
    public:
        /**
         * @brief Saves the results of a completed game.
         * @param finalScore The player's final score.
         * @param won True if the player reached the 2048 tile, false otherwise.
         */
        static void save_game(int finalScore, bool won);

        /**
         * @brief Loads the all-time high score from the data file.
         * @return The high score, or 0 if no scores have been saved yet.
         */
        static int load_high_score();

    private:
        // The name of the file used to store scores.
        static const std::string kScoreFileName;
    };

}  // namespace tfe::score
