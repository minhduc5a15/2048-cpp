#pragma once
#include <string>
#include <fstream>
#include "core/board.h"

namespace tfe::train {

    class Trainer {
    public:
        // Alpha: Learning rate (default 0.0025f)
        explicit Trainer(float alpha = 0.0025f);

        // Runs the training loop
        void run(int episodes);

        // Saves weights to a binary file (in a game-compatible format)
        static void saveWeights(const std::string& filepath);

    private:
        float alpha_;
        tfe::core::Board board_;

        // --- Logging ---
        std::ofstream logFile_;
        void initLogFile();
        void logEpisode(int episode, int score, int maxTile, int steps, double durationMs);

        // --- RL Helpers ---
        // Finds the best move (Greedy Policy based on current weights)
        static tfe::core::Direction findBestMove(const tfe::core::Board& board);

        // Calculates the state value V(s)
        static float evaluate(uint64_t boardState);

        // Updates weights (TD Update)
        void updateWeights(uint64_t boardState, float delta) const;

        // Helper to get the max tile
        static int getMaxTile(const tfe::core::Board& board);
    };
}
