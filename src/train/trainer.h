#pragma once
#include <string>
#include <fstream>
#include "core/board.h"

namespace tfe::train {

    /**
     * @class Trainer
     * @brief Implements a Reinforcement Learning agent using Temporal Difference (TD) Learning.
     *
     * The goal is to learn the optimal weights for the N-Tuple Network (lookup tables)
     * by playing millions of games against itself.
     * 
     * Algorithm: TD(0) Learning (specifically Sarsa/Q-Learning variant for 2048)
     * Value Function Approximation: V(state) = Sum(Table[feature])
     */
    class Trainer {
    public:
        /**
         * @brief Constructs a new Trainer.
         * @param alpha The learning rate (step size). 
         *              High alpha = learns fast but unstable. 
         *              Low alpha = stable but slow convergence.
         */
        explicit Trainer(float alpha = 0.0001f);

        /**
         * @brief Executes the training loop for a specified number of episodes.
         * @param episodes Total number of games to play.
         */
        void run(int episodes);

        /**
         * @brief Saves the learned weights to a binary file.
         * The format must match what `LookupTable::loadWeights` expects.
         * @param filepath Output path.
         */
        static void saveWeights(const std::string& filepath);

    private:
        float alpha_;
        tfe::core::Board board_;

        // --- Logging & Statistics ---
        std::ofstream logFile_;
        void initLogFile();
        void logChunk(int startEp, int endEp, double avgScore, int maxScore, int maxTile, double avgSteps, double avgTime, int elapsed);

        // Aggregated stats for current chunk
        long long chunkScoreSum_ = 0;
        int chunkMaxScore_ = 0;
        int chunkMaxTile_ = 0;
        long long chunkStepsSum_ = 0;
        double chunkDurationSum_ = 0.0;

        // --- RL Core Methods ---
        
        /**
         * @brief Chooses the best move based on the CURRENT learned weights.
         * This implements a Greedy Policy (always exploitation, no exploration noise needed for 2048 usually).
         */
        static tfe::core::Direction findBestMove(const tfe::core::Board& board);

        /**
         * @brief Computes V(s), the estimated value of a board state.
         * Sums up the weights from all active features (rows, cols, squares).
         */
        static float evaluate(uint64_t boardState);

        /**
         * @brief Performs the Temporal Difference update.
         * Updates the weights associated with the current state features by a fraction (alpha) of the error (delta).
         * @param boardState The state to update.
         * @param delta The TD Error: (Reward + V(next_state)) - V(current_state).
         */
        void updateWeights(uint64_t boardState, float delta) const;

        /**
         * @brief Helper to find the maximum tile value on the board (for stats).
         */
        static int getMaxTile(const tfe::core::Board& board);
    };
}
