#include "score-manager.h"

#include <nlohmann/json.hpp>
#include "platform.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace tfe::score {

    using json = nlohmann::json;

    // Helper to get the full, platform-specific path for the score file.
    std::filesystem::path getScoreFilePath() {
        std::filesystem::path userDataPath = tfe::platform::get_user_data_directory();
        if (userDataPath.empty()) {
            // Fallback to current directory if we can't get a user data path
            return "scores.json";
        }

        // Create a dedicated directory for our app inside the user data folder
        std::filesystem::path appDataPath = userDataPath / "2048-cpp";
        std::filesystem::create_directories(appDataPath); // a-cpp

        return appDataPath / "scores.json";
    }


    // Helper to get current timestamp as a string
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
            return 0; // File doesn't exist, so no high score yet.
        }

        int highScore = 0;
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            try {
                json game = json::parse(line);
                if (game.contains("score") && game["score"].is_number_integer()) {
                    if (game["score"] > highScore) {
                        highScore = game["score"];
                    }
                }
            } catch (json::parse_error& e) {
                std::cerr << "Warning: Could not parse a line in score file. Line: " << line << std::endl;
                // Continue to next line
            }
        }
        return highScore;
    }

    void ScoreManager::save_game(int finalScore, bool won) {
        const auto scorePath = getScoreFilePath();
        const int currentHighScore = load_high_score();
        const bool isNewRecord = (finalScore > currentHighScore);

        json newGame;
        newGame["timestamp"] = getCurrentTimestamp();
        newGame["score"] = finalScore;
        newGame["achieved_2048"] = won;
        newGame["is_new_highscore"] = isNewRecord;

        // Open the file in append mode.
        std::ofstream outputFile(scorePath, std::ios::app);
        if (outputFile.is_open()) {
            // Write the new game object as a single line, followed by a newline.
            outputFile << newGame.dump() << std::endl;
        } else {
            std::cerr << "Error: Could not open " << scorePath << " for writing." << std::endl;
        }
    }

}  // namespace tfe::score
