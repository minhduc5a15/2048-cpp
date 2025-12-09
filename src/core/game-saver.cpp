#include "game-saver.h"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace tfe::core {

    static std::string getSavePath() { return "savegame.json"; }

    void GameSaver::save(const GameState& state) {
        json j;
        j["score"] = state.score;
        j["board"] = state.board;  // json hỗ trợ uint64_t
        if (std::ofstream file(getSavePath()); file.is_open()) {
            file << j.dump(4);
        }
    }

    std::optional<GameState> GameSaver::load() {
        if (!std::filesystem::exists(getSavePath())) return std::nullopt;
        std::ifstream file(getSavePath());
        if (!file.is_open()) return std::nullopt;

        try {
            json j;
            file >> j;
            GameState state;
            state.score = j.at("score").get<int>();
            state.board = j.at("board").get<Bitboard>();
            return state;
        } catch (...) {
            return std::nullopt;
        }
    }

    void GameSaver::clearSave() {
        if (std::filesystem::exists(getSavePath())) {
            std::filesystem::remove(getSavePath());
        }
    }

    bool GameSaver::hasSave() { return std::filesystem::exists(getSavePath()); }

}  // namespace tfe::core
