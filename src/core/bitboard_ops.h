#pragma once
#include "types.h"
#include <utility>

namespace tfe::core::BitboardOps {

    /**
     * @brief Transposes the 4x4 bitboard (swaps rows and columns).
     * 
     * @param b The input bitboard.
     * @return The transposed bitboard.
     */
    Bitboard transpose64(Bitboard b);

    /**
     * @brief Counts the number of empty tiles (value 0) on the board.
     * 
     * @param b The input bitboard.
     * @return The number of empty tiles.
     */
    int countEmpty(Bitboard b);

    /**
     * @brief Counts the number of distinct tile values on the board.
     * 
     * @param b The input bitboard.
     * @return The number of distinct non-zero tiles.
     */
    int countDistinctTiles(Bitboard b);

    /**
     * @brief Executes a move in the given direction.
     * 
     * Applies the move logic (shift and merge) to the board.
     * Does NOT spawn new tiles.
     * 
     * @param board The current board state.
     * @param dir The direction to move.
     * @return A pair containing the new board state and the score gained from merges.
     */
    std::pair<Bitboard, int> executeMove(Bitboard board, Direction dir);

}
