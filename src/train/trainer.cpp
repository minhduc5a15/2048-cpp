#include "trainer.h"

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>

#include "core/lookup_table.h"

namespace tfe::train {

    using namespace tfe::core;

    // Helper: Transpose 64-bit board
    static inline uint64_t transpose64(const uint64_t x) {
        const uint64_t a1 = x & 0xF0F00F0FF0F00F0FULL;
        const uint64_t a2 = x & 0x0000F0F00000F0F0ULL;
        const uint64_t a3 = x & 0x0F0F00000F0F0000ULL;
        const uint64_t a = a1 | (a2 << 12) | (a3 >> 12);
        const uint64_t b1 = a & 0xFF00FF0000FF00FFULL;
        const uint64_t b2 = a & 0x00FF00FF00000000ULL;
        const uint64_t b3 = a & 0x00000000FF00FF00ULL;
        return b1 | (b2 >> 24) | (b3 << 24);
    }

    // Helper: Get the index of a 2x2 square (Copied from ai_solver.cpp)
    static inline uint16_t getSquareIndex(const uint64_t board, const int shift) {
        uint16_t idx = 0;
        idx |= (board >> shift) & 0xF;
        idx |= ((board >> (shift + 4)) & 0xF) << 4;
        idx |= ((board >> (shift + 16)) & 0xF) << 8;
        idx |= ((board >> (shift + 20)) & 0xF) << 12;
        return idx;
    }

    Trainer::Trainer(float alpha) : alpha_(alpha), board_(4) {
        // Ensure LookupTable is initialized (so the array is ready for writing)
        LookupTable::init();

        LookupTable::loadWeights("build/bin/tuple_weights.bin");

        initLogFile();
    }

    void Trainer::initLogFile() {
        if (!std::filesystem::exists("logs")) {
            std::filesystem::create_directory("logs");
        }

        // Create filename based on timestamp
        const auto now = std::chrono::system_clock::now();
        const auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << "logs/train_cpp_" << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S") << ".csv";

        logFile_.open(ss.str());
        // Write CSV Header
        logFile_ << "Episode,Score,MaxTile,Steps,TimeMs\n";

        std::cout << "Logging training to: " << ss.str() << std::endl;
    }

    void Trainer::logEpisode(const int episode, const int score, const int maxTile, const int steps, const double durationMs) {
        logFile_ << episode << "," << score << "," << maxTile << "," << steps << "," << std::fixed << std::setprecision(2) << durationMs << "\n";
    }

    int Trainer::getMaxTile(const Board& board) {
        const uint64_t b = board.getState().board;

        int maxExp = 0;

        // 2. Iterate through 16 cells (4 bits each)
        for (int i = 0; i < 16; ++i) {
            // Get the 4 bits at position i
            if (const int currentExp = (b >> (i * 4)) & 0xF; currentExp > maxExp) {
                maxExp = currentExp;
            }
        }

        // 3. Convert exponent to actual value (e.g., 11 -> 2048)
        return (maxExp == 0) ? 0 : (1 << maxExp);
    }

    float Trainer::evaluate(const uint64_t boardState) {
        float score = 0;
        // 1. Rows
        for (int i = 0; i < 64; i += 16) score += LookupTable::heuristicTable[(boardState >> i) & 0xFFFF];

        // 2. Cols
        const uint64_t t = transpose64(boardState);
        for (int i = 0; i < 64; i += 16) score += LookupTable::heuristicTable[(t >> i) & 0xFFFF];

        // 3. Squares
        constexpr int shifts[] = {0, 4, 8, 16, 20, 24, 32, 36, 40};
        for (const int s : shifts) score += LookupTable::squareTable[getSquareIndex(boardState, s)];

        return score;
    }

    void Trainer::updateWeights(const uint64_t boardState, const float delta) const {
        const float change = alpha_ * delta;

        // Update Rows
        for (int i = 0; i < 64; i += 16) {
            LookupTable::heuristicTable[(boardState >> i) & 0xFFFF] += change;
        }

        // Update Cols
        const uint64_t t = transpose64(boardState);
        for (int i = 0; i < 64; i += 16) {
            LookupTable::heuristicTable[(t >> i) & 0xFFFF] += change;
        }

        // Update Squares
        constexpr int shifts[] = {0, 4, 8, 16, 20, 24, 32, 36, 40};
        for (const int s : shifts) {
            LookupTable::squareTable[getSquareIndex(boardState, s)] += change;
        }
    }

    Direction Trainer::findBestMove(const Board& board) {
        float bestScore = -std::numeric_limits<float>::max();
        auto bestDir = Direction::Up;
        bool found = false;

        // Try only 4 directions
        constexpr Direction dirs[] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};

        // Get the original state
        GameState originalState = board.getState();
        const int originalScore = originalState.score;

        // Since our Board class changes its internal state,
        // we need to either copy it to a draft or use a save/load state mechanism.
        // Here, we use a temporary board (like temp_board in Python) for optimization.
        // However, the C++ Board class is quite lightweight (contains only a u64), so copying the Board is also fine.

        for (const auto dir : dirs) {
            // Create a quick copy
            Board tempBoard = board;

            const bool changed = tempBoard.move(dir);
            if (changed) {
                const float stateVal = evaluate(tempBoard.getState().board);
                const float reward = static_cast<float>(tempBoard.getScore() - originalScore);

                if (const float total = reward + stateVal; total > bestScore) {
                    bestScore = total;
                    bestDir = dir;
                    found = true;
                }
            }
        }

        // If no move is possible, return Up (will break the outer loop)
        return found ? bestDir : Direction::Up;
    }

    void Trainer::run(const int episodes) {
        std::cout << "Starting C++ Training for " << episodes << " episodes..." << std::endl;

        const auto startTime = std::chrono::high_resolution_clock::now();
        int maxScoreBatch = 0;
        int globalMaxScore = 0;

        const float startAlpha = alpha_;

        for (int ep = 1; ep <= episodes; ++ep) {
            const float progress = episodes;
            alpha_ = std::max(0.0001f, startAlpha * (1.0f - progress));
            auto epStart = std::chrono::high_resolution_clock::now();

            board_.reset();
            int steps = 0;

            while (!board_.isGameOver()) {
                // 1. Choose a move
                const Direction bestDir = findBestMove(board_);

                const int prevScore = board_.getScore();
                const uint64_t currentState = board_.getState().board;

                // 2. Execute the move
                if (const bool changed = board_.move(bestDir); !changed) break;  // Game Over or stuck

                steps++;

                // 3. Calculate Reward & Next State
                const int reward = board_.getScore() - prevScore;
                const uint64_t nextState = board_.getState().board;

                // 4. TD Learning Update
                const float currentVal = evaluate(currentState);
                float target = 0;

                if (!board_.isGameOver()) {
                    target = static_cast<float>(reward) + evaluate(nextState);
                }
                // If Game Over, target = 0 (or the final reward)

                const float delta = target - currentVal;
                updateWeights(currentState, delta);
            }

            // --- End Episode ---
            auto epEnd = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration = epEnd - epStart;

            const int finalScore = board_.getScore();
            if (finalScore > maxScoreBatch) maxScoreBatch = finalScore;

            logEpisode(ep, finalScore, getMaxTile(board_), steps, duration.count());

            // Log to the console every 1000 games (C++ is fast, so log less frequently)
            if (ep % 1000 == 0) {
                const auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(epEnd - startTime).count();
                const double speed = static_cast<double>(totalTime) / ep;

                std::cout << "Ep " << ep << "/" << episodes << " | MaxScore: " << maxScoreBatch << " | Last: " << finalScore << " | AvgSpeed: " << std::fixed
                          << std::setprecision(2) << speed << "ms/game" << std::endl;

                maxScoreBatch = 0;
                // Auto save
                saveWeights("build/bin/tuple_weights.bin");
            }
        }

        logFile_.close();
        saveWeights("build/bin/tuple_weights.bin");
        std::cout << "Training Complete!" << std::endl;
    }

    void Trainer::saveWeights(const std::string& filepath) {
        std::ofstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error saving weights to " << filepath << std::endl;
            return;
        }

        // IMPORTANT: Write the element count header (65536) to match the loading code
        constexpr uint32_t count = TABLE_SIZE;
        file.write(reinterpret_cast<const char*>(&count), sizeof(count));

        // Write Row Weights
        file.write(reinterpret_cast<const char*>(LookupTable::heuristicTable), count * sizeof(float));

        // Write Square Weights
        file.write(reinterpret_cast<const char*>(LookupTable::squareTable), count * sizeof(float));

        // std::cout << "Saved weights to " << filepath << std::endl;
    }
}  // namespace tfe::train
