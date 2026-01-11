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
         * Uses Dynamic Depth Search with Cumulative Probability Pruning.
         * The depth is determined by the number of distinct tiles on the board.
         * 
         * @param board The current game board.
         * @return The best calculated Direction.
         */
        static Direction findBestMove(const Board& board);

    private:
        /**
         * @brief Evaluates the "goodness" of a static board state.
         * 
         * Uses the pre-calculated heuristic tables.
         * 
         * @param board The board state to evaluate.
         * @return A floating-point score (higher is better).
         */
        static float evaluateBoard(Bitboard board);

        /**
         * @brief The core recursive Expectimax function.
         * 
         * @param board The current board state.
         * @param depth Current depth of the search.
         * @param depthLimit The maximum depth allowed for this search.
         * @param isPlayerTurn True if it's the player's move, False if it's the environment's spawn.
         * @param cprob The cumulative probability of reaching this node. Used for pruning.
         * @return The expected score of the node.
         */
        static float expectimax(Bitboard board, int depth, int depthLimit, bool isPlayerTurn, float cprob);
    };
}  // namespace tfe::core