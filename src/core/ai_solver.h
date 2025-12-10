#pragma once
#include "board.h"

namespace tfe::core {

    class AISolver {
    public:
        /**
         * @brief Finds the best move for the current board state using Expectimax.
         * @param board The current board state.
         * @param depth Search depth (default 4 is reasonably strong).
         * @return The best direction to move.
         */
        static Direction findBestMove(const Board& board, int depth = 4); 

    private:
        /**
         * @brief Evaluates the current board score using the LookupTable.
         * @param board The bitboard to evaluate.
         * @return The heuristic score.
         */
        static float evaluateBoard(Bitboard board);
        
        /**
         * @brief Recursively calculates the expectimax score.
         * @param board Current bitboard state.
         * @param depth Current recursion depth.
         * @param isPlayerTurn True if it's the player's turn (Max node), False for chance node.
         * @param cumulativeProb Cumulative probability of reaching this state (for pruning).
         * @return The expected score.
         */
        static float expectimax(Bitboard board, int depth, bool isPlayerTurn, float cumulativeProb);
    };
}