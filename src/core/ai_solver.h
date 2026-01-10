#pragma once
#include "board.h"
#include <chrono>

namespace tfe::core {

    /**
     * @class AISolver
     * @brief Implements the AI logic using the Expectimax algorithm.
     *
     * The 2048 game involves randomness (random tile spawns), so Minimax is not suitable.
     * Expectimax treats the "opponent" (the computer spawning tiles) not as minimizing our score,
     * but as a chance node that averages the possible outcomes based on probability.
     */
    class AISolver {
    public:
        /**
         * @brief Finds the best move for the current board state.
         * 
         * Uses Iterative Deepening Expectimax. It starts searching at depth 1, then depth 2,
         * and so on, until the time limit is reached.
         * 
         * @param board The current game board.
         * @param depth The maximum depth limit (soft limit). The time limit usually cuts it off earlier.
         * @return The best calculated Direction.
         */
        static Direction findBestMove(const Board& board, int depth = 10);

    private:
        /**
         * @brief Evaluates the "goodness" of a static board state.
         * 
         * Uses the pre-calculated heuristic tables (weighted sum of monotonicity, 
         * empty cells, etc.) or loaded neural network weights.
         * 
         * @param board The board state to evaluate.
         * @return A floating-point score (higher is better).
         */
        static float evaluateBoard(Bitboard board);

        /**
         * @brief The core recursive Expectimax function.
         * 
         * @param board The current board state.
         * @param depth Remaining depth to search.
         * @param isPlayerTurn True if it's the player's move, False if it's the environment's spawn.
         * @param cumulativeProb Optimization: The probability of reaching this node. Used to prune unlikely branches.
         * @param deadline The exact time point when calculation must stop.
         * @param nodesVisited Counter for performance metrics and checking time periodically.
         * @return The expected score of the node.
         */
        static float expectimax(Bitboard board, int depth, bool isPlayerTurn, float cumulativeProb,
                                std::chrono::high_resolution_clock::time_point deadline, int& nodesVisited);
    };
}  // namespace tfe::core