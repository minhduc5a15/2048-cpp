#include "ai_solver.h"

#include <algorithm>
#include <limits>
#include <vector>

#include "lookup_table.h"
#include "transposition_table.h"

namespace tfe::core {
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
    /**
     * @brief Extracts a column from the bitboard and packs it into a 16-bit row format.
     *
     * This allows us to use row-based lookup tables for column evaluation without
     * needing to transpose the entire board.
     *
     * @param board The 64-bit board.
     * @param c The column index (0-3).
     * @return 16-bit integer representing the column tiles.
     */
    static inline Row extractColumn(const Bitboard board, const int c) {
        // Mask out the specific column bits and shift them to positions 0, 12, 24, 36...
        uint64_t ret = (board >> c) & 0x000F000F000F000FULL;

        // Compact the bits into the lower 16 bits
        ret |= (ret >> 12);
        ret |= (ret >> 24);
        return static_cast<Row>(ret & 0xFFFF);
    }

    float AISolver::evaluateBoard(const Bitboard board) {
        float score = 0;

        // 1. Evaluate Rows (Horizontal patterns)
        for (int i = 0; i < 64; i += 16) score += LookupTable::heuristicTable[(board >> i) & 0xFFFF];

        // 2. Evaluate Columns (Vertical patterns)
        // We use extractColumn to treat columns as rows and reuse the heuristic table.
        for (int c = 0; c < 4; ++c) {
            score += LookupTable::heuristicTable[extractColumn(board, c)];
        }

        return score;
    }

    /**
     * @brief Counts the number of distinct tile values on the board.
     * Used to calculate the dynamic search depth.
     */
    static int countDistinctTiles(const Bitboard board) {
        int count = 0;
        uint32_t seenMask = 0;
        for (int i = 0; i < 16; ++i) {
            int val = (board >> (i * 4)) & 0xF;
            if (val > 0 && !((seenMask >> val) & 1)) {
                seenMask |= (1 << val);
                count++;
            }
        }
        return count;
    }

    Direction AISolver::findBestMove(const Board& board) {
        const Bitboard currentBoard = board.getState().board;

        // Khởi tạo bestMove là một hướng bất kỳ, nhưng quan trọng là bestScore phải cực nhỏ
        auto bestMove = Direction::Up;
        float bestScore = -std::numeric_limits<float>::max();

        TranspositionTable::instance().clear();

        int distinctTiles = countDistinctTiles(currentBoard);
        int depthLimit = std::max(3, distinctTiles - 2);

        // Pre-calculate transposed board for Up/Down moves
        const Bitboard currentBoardT = transpose64(currentBoard);

        for (const auto dir : {Direction::Up, Direction::Down, Direction::Left, Direction::Right}) {
            Bitboard nextBoard = 0;
            bool changed = false;

            if (dir == Direction::Left) {
                // Giữ nguyên logic cũ
                for (int r = 0; r < 4; ++r) {
                    const Row row = (currentBoard >> (r * 16)) & 0xFFFF;
                    const Row newRow = LookupTable::moveLeftTable[row];
                    if (row != newRow) changed = true;
                    nextBoard |= (static_cast<Bitboard>(newRow) << (r * 16));
                }
            } else if (dir == Direction::Right) {
                // Giữ nguyên logic cũ
                for (int r = 0; r < 4; ++r) {
                    const Row row = (currentBoard >> (r * 16)) & 0xFFFF;
                    const Row newRow = LookupTable::moveRightTable[row];
                    if (row != newRow) changed = true;
                    nextBoard |= (static_cast<Bitboard>(newRow) << (r * 16));
                }
            } else if (dir == Direction::Up) {
                // --- SỬA ĐỔI: Dùng Transpose logic ---
                // Move Up trên bàn cờ thường = Move Left trên bàn cờ đã xoay
                Bitboard nextBoardT = 0;
                for (int r = 0; r < 4; ++r) {
                    const Row row = (currentBoardT >> (r * 16)) & 0xFFFF;
                    const Row newRow = LookupTable::moveLeftTable[row];
                    if (row != newRow) changed = true;
                    nextBoardT |= (static_cast<Bitboard>(newRow) << (r * 16));
                }
                if (changed) nextBoard = transpose64(nextBoardT);
            } else {  // Down
                // --- SỬA ĐỔI: Dùng Transpose logic ---
                // Move Down trên bàn cờ thường = Move Right trên bàn cờ đã xoay
                Bitboard nextBoardT = 0;
                for (int r = 0; r < 4; ++r) {
                    const Row row = (currentBoardT >> (r * 16)) & 0xFFFF;
                    const Row newRow = LookupTable::moveRightTable[row];
                    if (row != newRow) changed = true;
                    nextBoardT |= (static_cast<Bitboard>(newRow) << (r * 16));
                }
                if (changed) nextBoard = transpose64(nextBoardT);
            }

            if (changed) {
                float score = expectimax(nextBoard, 1, depthLimit, false, 1.0f);
                // Cập nhật nước đi tốt nhất dựa trên điểm số thực tế
                if (score > bestScore) {
                    bestScore = score;
                    bestMove = dir;
                }
            }
        }

        return bestMove;
    }

    float AISolver::expectimax(const Bitboard board, const int depth, const int depthLimit, const bool isPlayerTurn, const float cprob) {
        // Base cases:
        // 1. Probability is too low (Pruning): This branch is unlikely to happen.
        // 2. Depth limit reached.
        if (cprob < 0.0001f || depth >= depthLimit) {
            return evaluateBoard(board);
        }

        // Transposition Table Lookup (Memoization)
        // We use "remaining depth" logic for the transposition table to be compatible with its interface.
        // TT expects: entry.depth >= needed_depth.
        // Our needed_depth (remaining) is depthLimit - depth.
        // Note: We only cache Environment nodes (Chance nodes) usually, or we can cache both.
        // Here we cache when it's NOT player turn (i.e., before chance node evaluation).
        if (!isPlayerTurn) {
            float cachedScore;
            // Map current depth logic to "remaining depth" for TT compatibility
            int remainingDepth = depthLimit - depth;
            if (TranspositionTable::instance().get(board, remainingDepth, cachedScore)) return cachedScore;
        }

        if (isPlayerTurn) {  // MAX Node (Player tries to maximize score)
            float maxVal = -std::numeric_limits<float>::max();
            bool canMove = false;

            // Try all 4 moves. Loops unrolled for slight performance gain.

            // LEFT
            {
                Bitboard next = 0;
                for (int r = 0; r < 64; r += 16) next |= static_cast<Bitboard>(LookupTable::moveLeftTable[(board >> r) & 0xFFFF]) << r;
                if (next != board) {
                    canMove = true;
                    // Player move transitions to Chance node (isPlayerTurn = false).
                    // Depth increases on player move.
                    float val = expectimax(next, depth + 1, depthLimit, false, cprob);
                    if (val > maxVal) maxVal = val;
                }
            }
            // RIGHT
            {
                Bitboard next = 0;
                for (int r = 0; r < 64; r += 16) next |= static_cast<Bitboard>(LookupTable::moveRightTable[(board >> r) & 0xFFFF]) << r;
                if (next != board) {
                    canMove = true;
                    float val = expectimax(next, depth + 1, depthLimit, false, cprob);
                    if (val > maxVal) maxVal = val;
                }
            }
            // UP
            {
                Bitboard next = 0;
                for (int c = 0; c < 4; ++c) next |= LookupTable::colUpTable[extractColumn(board, c)] << c;
                if (next != board) {
                    canMove = true;
                    float val = expectimax(next, depth + 1, depthLimit, false, cprob);
                    if (val > maxVal) maxVal = val;
                }
            }
            // DOWN
            {
                Bitboard next = 0;
                for (int c = 0; c < 4; ++c) next |= LookupTable::colDownTable[extractColumn(board, c)] << c;
                if (next != board) {
                    canMove = true;
                    float val = expectimax(next, depth + 1, depthLimit, false, cprob);
                    if (val > maxVal) maxVal = val;
                }
            }

            return canMove ? maxVal : 0;  // Return 0 (effectively -inf penalty relative to others) if no moves possible
        }

        // CHANCE Node (Environment spawns a tile)
        // Calculates the weighted average of all possible spawn outcomes.
        float totalScore = 0;
        int emptyCount = 0;

        // Iterate over all 16 positions
        for (int i = 0; i < 16; ++i) {
            if (((board >> (i * 4)) & 0xF) == 0) {
                emptyCount++;
                // 90% chance of spawning 2 (Value 1)
                // Chance node transitions to Player node (isPlayerTurn = true).
                // Depth does NOT increase on spawn (only on player moves).
                totalScore += 0.9f * expectimax(board | (static_cast<Bitboard>(1) << (i * 4)), depth, depthLimit, true, cprob * 0.9f);
                // 10% chance of spawning 4 (Value 2)
                totalScore += 0.1f * expectimax(board | (static_cast<Bitboard>(2) << (i * 4)), depth, depthLimit, true, cprob * 0.1f);
            }
        }

        if (emptyCount == 0) return evaluateBoard(board);

        const float finalScore = totalScore / emptyCount;

        // Cache the result
        // Store with "remaining depth" logic
        TranspositionTable::instance().put(board, depthLimit - depth, finalScore);

        return finalScore;
    }
}  // namespace tfe::core