#include "score-manager.h"

#include <nlohmann/json.hpp>
#include "platform.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace tfe::score {

    using json = nlohmann::json;

    /**
     * @brief Determines the platform-specific path for the scores file.
     * 
     * - Windows: %APPDATA%/2048-cpp/scores.json
     * - Linux/macOS: ~/.local/share/2048-cpp/scores.json
     * 
     * Ensures that the directory exists before returning the path.
     * Falls back to the current directory ("scores.json") if the user data path is unavailable.
     * 
     * @return std::filesystem::path The full path to the scores file.
     */
    std::filesystem::path getScoreFilePath() {
        const std::filesystem::path userDataPath = tfe::platform::get_user_data_directory();
        if (userDataPath.empty()) {
            // Fallback to current working directory
            return "scores.json";
        }

        // Create a sub-directory for this application to avoid cluttering the root data folder
        const std::filesystem::path appDataPath = userDataPath / "2048-cpp";
        
        // Ensure the directory structure exists
        std::filesystem::create_directories(appDataPath);

        return appDataPath / "scores.json";
    }

    /**
     * @brief Generates a formatted timestamp string (YYYY-MM-DD HH:MM:SS).
     * Used for logging when a high score was achieved.
     */
    std::string getCurrentTimestamp() {
        const auto now = std::chrono::system_clock::now();
        const auto time_t_now = std::chrono::system_clock::to_time_t(now);
        char buffer[30];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&time_t_now));
        return buffer;
    }

    int ScoreManager::load_high_score() {
        const auto scorePath = getScoreFilePath();
        std::ifstream file(scorePath);
        if (!file.is_open()) {
            // If the file doesn't exist yet, the high score is 0.
            return 0;
        }

        int highScore = 0;
        std::string line;
        
        // Read the file line by line (JSON Lines format) to find the maximum score.
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            try {
                // Parse each line as a separate JSON object
                if (json game = json::parse(line); game.contains("score") && game["score"].is_number_integer()) {
                    if (game["score"] > highScore) {
                        highScore = game["score"];
                    }
                }
            } catch (json::parse_error& e) {
                // Silently ignore corrupted lines to prevent crashing
                std::cerr << "[ScoreManager] Warning: Skipped corrupted line in score file." << std::endl;
            }
        }
        return highScore;
    }

    void ScoreManager::save_game(int finalScore, bool won) {
        const auto scorePath = getScoreFilePath();
        const int currentHighScore = load_high_score();
        const bool isNewRecord = (finalScore > currentHighScore);

        // Construct the JSON object for this game session
        json newGame;
        newGame["timestamp"] = getCurrentTimestamp();
        newGame["score"] = finalScore;
        newGame["achieved_2048"] = won;
        newGame["is_new_highscore"] = isNewRecord;

        // Open file in APPEND mode so we keep a history of games
        if (std::ofstream outputFile(scorePath, std::ios::app); outputFile.is_open()) {
            // Write compact JSON on a single line
            outputFile << newGame.dump() << std::endl;
        } else {
            std::cerr << "[ScoreManager] Error: Could not save score to " << scorePath << std::endl;
        }
    }

}  // namespace tfe::score
