#include "ai_solver.h"

#include <algorithm>
#include <limits>

#include "config.h"
#include "lookup_table.h"

namespace tfe::core {

    // Helper: Transpose (copied from board.cpp)
    static inline Bitboard transpose64(const Bitboard x) {
        const Bitboard a1 = x & 0xF0F00F0FF0F00F0FULL;
        const Bitboard a2 = x & 0x0000F0F00000F0F0ULL;
        const Bitboard a3 = x & 0x0F0F00000F0F0000ULL;
        const Bitboard a = a1 | (a2 << 12) | (a3 >> 12);
        const Bitboard b1 = a & 0xFF00FF0000FF00FFULL;
        const Bitboard b2 = a & 0x00FF00FF00000000ULL;
        const Bitboard b3 = a & 0x00000000FF00FF00ULL;
        return b1 | (b2 >> 24) | (b3 << 24);
    }

    // Helper: Count empty cells (to optimize probabilities)
    static int countEmpty(const Bitboard board) {
        int count = 0;
        for (int i = 0; i < 16; ++i) {
            if (((board >> (i * 4)) & 0xF) == 0) count++;
        }
        return count;
    }

    // Evaluate board based on LookupTable (trained weights)
    float AISolver::evaluateBoard(const Bitboard board) {
        // Evaluate 4 horizontal rows
        float score = LookupTable::heuristicTable[(board >> 0) & 0xFFFF] + LookupTable::heuristicTable[(board >> 16) & 0xFFFF] +
                      LookupTable::heuristicTable[(board >> 32) & 0xFFFF] + LookupTable::heuristicTable[(board >> 48) & 0xFFFF];

        // Evaluate 4 vertical columns (Transpose)
        const Bitboard t = transpose64(board);
        score += LookupTable::heuristicTable[(t >> 0) & 0xFFFF] + LookupTable::heuristicTable[(t >> 16) & 0xFFFF] + LookupTable::heuristicTable[(t >> 32) & 0xFFFF] +
                 LookupTable::heuristicTable[(t >> 48) & 0xFFFF];

        return score;
    }

    Direction AISolver::findBestMove(const Board& boardData, int depth) {
        const Bitboard currentBoard = boardData.getState().board;
        float bestScore = -std::numeric_limits<float>::max();
        int bestMove = -1;

        // Try 4 directions: Up, Down, Left, Right
        // Enum order: Up=0, Down=1, Left=2, Right=3

        constexpr Direction dirs[4] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};

        for (int i = 0; i < 4; ++i) {
            Bitboard nextBoard = currentBoard;
            bool changed = false;

            // --- Move Simulation Logic (Optimized for Bitboard) ---
            const bool needTranspose = (dirs[i] == Direction::Up || dirs[i] == Direction::Down);
            if (needTranspose) nextBoard = transpose64(nextBoard);

            Bitboard tempBoard = 0;
            for (int r = 0; r < 4; ++r) {
                const Row row = (nextBoard >> (r * 16)) & Config::ROW_MASK;
                Row newRow;
                // Left or Up (transposed to Left) use moveLeftTable
                if (dirs[i] == Direction::Left || dirs[i] == Direction::Up)
                    newRow = LookupTable::moveLeftTable[row];
                else
                    newRow = LookupTable::moveRightTable[row];

                tempBoard |= (static_cast<Bitboard>(newRow) << (r * 16));
            }
            if (tempBoard != nextBoard) changed = true;
            nextBoard = tempBoard;
            if (needTranspose) nextBoard = transpose64(nextBoard);
            // ---------------------------------------------------

            if (changed) {
                // Switch to computer's turn (spawn random tile) -> Expectation Node
                float score = expectimax(nextBoard, depth - 1, false, 1.0f);
                if (score > bestScore) {
                    bestScore = score;
                    bestMove = i;
                }
            }
        }

        if (bestMove != -1) return dirs[bestMove];
        return Direction::Up;  // Fallback
    }

    float AISolver::expectimax(const Bitboard board, const int depth, const bool isPlayerTurn, const float cumulativeProb) {
        // Pruning if probability is too low to matter
        if (cumulativeProb < 0.0001f || depth == 0) {
            return evaluateBoard(board);
        }

        if (isPlayerTurn) {  // Max Node (Player)
            float maxVal = -std::numeric_limits<float>::max();
            bool canMove = false;

            // Try 4 directions
            for (int dir = 0; dir < 4; ++dir) {
                Bitboard nextBoard = board;
                const bool needTranspose = (dir == 0 || dir == 1);  // Up/Down
                if (needTranspose) nextBoard = transpose64(nextBoard);

                Bitboard tempBoard = 0;
                for (int r = 0; r < 4; ++r) {
                    const Row row = (nextBoard >> (r * 16)) & 0xFFFF;
                    Row newRow;
                    if (dir == 0 || dir == 2)
                        newRow = LookupTable::moveLeftTable[row];
                    else
                        newRow = LookupTable::moveRightTable[row];
                    tempBoard |= (static_cast<Bitboard>(newRow) << (r * 16));
                }

                const bool changed = (tempBoard != nextBoard);
                nextBoard = tempBoard;
                if (needTranspose) nextBoard = transpose64(nextBoard);

                if (changed) {
                    canMove = true;
                    const float val = expectimax(nextBoard, depth - 1, false, cumulativeProb);
                    if (val > maxVal) maxVal = val;
                }
            }
            // If no moves possible -> Game Over -> Heavy penalty
            return canMove ? maxVal : 0;
        }
        // Chance Node (Computer spawns tile)
        float avgScore = 0;
        const int emptyCount = countEmpty(board);
        if (emptyCount == 0) return evaluateBoard(board);

        const float prob2 = 0.9f * cumulativeProb / emptyCount;  // Probability of spawning 2
        const float prob4 = 0.1f * cumulativeProb / emptyCount;  // Probability of spawning 4

        // Iterate over all empty cells
        for (int i = 0; i < 16; ++i) {
            if (((board >> (i * 4)) & 0xF) == 0) {
                // Spawn 2 (exponent 1)
                const Bitboard board2 = board | (static_cast<Bitboard>(1) << (i * 4));
                avgScore += 0.9f * expectimax(board2, depth - 1, true, prob2);

                // Spawn 4 (exponent 2)
                const Bitboard board4 = board | (static_cast<Bitboard>(2) << (i * 4));
                avgScore += 0.1f * expectimax(board4, depth - 1, true, prob4);
            }
        }
        // avgScore is already the weighted sum because probabilities were multiplied inside expectimax
        return avgScore;
    }
}  // namespace tfe::core