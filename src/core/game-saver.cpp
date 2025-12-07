#include "game-saver.h"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

#include "platform.h"

using json = nlohmann::json;

namespace tfe::core {

    // Helper: Get save file path (same directory as scores.json)
    std::filesystem::path getSavePath() {
        auto path = tfe::platform::get_user_data_directory();
        if (path.empty()) return "gamestate.json";  // Fallback

        path /= "2048-cpp";
        std::filesystem::create_directories(path);
        return path / "gamestate.json";
    }

    void GameSaver::save(const GameState& state) {
        json j;
        j["score"] = state.score;
        j["nextId"] = state.nextId;
        j["grid"] = state.grid;
        j["idGrid"] = state.idGrid;

        if (std::ofstream file(getSavePath()); file.is_open()) {
            file << j.dump(4);  // Pretty print with 4 indents
        }
    }

    std::optional<GameState> GameSaver::load() {
        std::filesystem::path path = getSavePath();
        if (!std::filesystem::exists(path)) {
            return std::nullopt;
        }

        std::ifstream file(path);
        if (!file.is_open()) return std::nullopt;

        try {
            json j;
            file >> j;

            GameState state;
            state.score = j.at("score").get<int>();
            state.nextId = j.at("nextId").get<int>();
            state.grid = j.at("grid").get<Grid>();
            state.idGrid = j.at("idGrid").get<std::vector<std::vector<int>>>();

            return state;
        } catch (...) {
            return std::nullopt;
        }
    }

    void GameSaver::clearSave() {
        if (const std::filesystem::path path = getSavePath(); std::filesystem::exists(path)) {
            std::filesystem::remove(path);
        }
    }

    bool GameSaver::hasSave() { return std::filesystem::exists(getSavePath()); }

}  // namespace tfe::core