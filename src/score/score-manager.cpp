#include "score-manager.h"

#include <nlohmann/json.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

// Helper macros to turn the PROJECT_ROOT_DIR macro into a string literal
#define STRINGIFY(s) #s
#define XSTRINGIFY(s) STRINGIFY(s)

namespace tfe::score {

    using json = nlohmann::json;

    // Construct the absolute path to the scores.json file
    const std::filesystem::path kScoreFilePath = std::filesystem::path(XSTRINGIFY(PROJECT_ROOT_DIR)) / "scores.json";

    // Helper to get current timestamp as a string
    std::string getCurrentTimestamp() {
        const auto now = std::chrono::system_clock::now();
        const auto time_t_now = std::chrono::system_clock::to_time_t(now);
        char buffer[30];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&time_t_now));
        return buffer;
    }

    int ScoreManager::load_high_score() {
        std::ifstream file(kScoreFilePath);
        if (!file.is_open()) {
            return 0; // File doesn't exist, so no high score yet.
        }

        json data;
        try {
            file >> data;
        } catch (json::parse_error& e) {
            std::cerr << "Error parsing score file: " << e.what() << std::endl;
            return 0; // If file is corrupt, treat as no high score.
        }

        int highScore = 0;
        if (data.contains("games") && data["games"].is_array()) {
            for (const auto& game : data["games"]) {
                if (game.contains("score") && game["score"].is_number_integer()) {
                    if (game["score"] > highScore) {
                        highScore = game["score"];
                    }
                }
            }
        }
        return highScore;
    }

    void ScoreManager::save_game(int finalScore, bool won) {
        json data;
        std::ifstream inputFile(kScoreFilePath);
        if (inputFile.is_open()) {
            try {
                inputFile >> data;
            } catch (json::parse_error& e) {
                // File is corrupt or empty, start with a fresh JSON object.
                data = json::object();
            }
        }

        if (!data.contains("games") || !data["games"].is_array()) {
            data["games"] = json::array();
        }

        const int currentHighScore = load_high_score();
        const bool isNewRecord = (finalScore > currentHighScore);

        json newGame;
        newGame["timestamp"] = getCurrentTimestamp();
        newGame["score"] = finalScore;
        newGame["achieved_2048"] = won;
        newGame["is_new_highscore"] = isNewRecord;

        data["games"].push_back(newGame);

        std::ofstream outputFile(kScoreFilePath);
        if (outputFile.is_open()) {
            outputFile << data.dump(4); // pretty print with 4 spaces
        } else {
            std::cerr << "Error: Could not open " << kScoreFilePath << " for writing." << std::endl;
        }
    }

}  // namespace tfe::score
