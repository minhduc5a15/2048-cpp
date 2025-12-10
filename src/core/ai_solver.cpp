#include "ai_solver.h"

#include <chrono>
#include <limits>

#include "config.h"
#include "lookup_table.h"
#include "transposition_table.h"

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

    // Helper: Evaluate 2x2 square based on LookupTable

    static inline uint16_t getSquareIndex(const Bitboard board, const int shift) {
        // Get the 2x2 square starting at bit `shift`
        // Cell 1: shift
        // Cell 2: shift + 4 (right)
        // Cell 3: shift + 16 (below)
        // Cell 4: shift + 20 (bottom right)

        uint16_t idx = 0;
        idx |= (board >> shift) & 0xF;
        idx |= ((board >> (shift + 4)) & 0xF) << 4;
        idx |= ((board >> (shift + 16)) & 0xF) << 8;
        idx |= ((board >> (shift + 20)) & 0xF) << 12;
        return idx;
    }

    // Evaluate board based on LookupTable (trained weights)
    float AISolver::evaluateBoard(const Bitboard board) {
        float score = 0;

        // 1. Evaluate Rows
        score += LookupTable::heuristicTable[(board >> 0) & 0xFFFF];
        score += LookupTable::heuristicTable[(board >> 16) & 0xFFFF];
        score += LookupTable::heuristicTable[(board >> 32) & 0xFFFF];
        score += LookupTable::heuristicTable[(board >> 48) & 0xFFFF];

        // 2. Evaluate Columns
        const Bitboard t = transpose64(board);
        score += LookupTable::heuristicTable[(t >> 0) & 0xFFFF];
        score += LookupTable::heuristicTable[(t >> 16) & 0xFFFF];
        score += LookupTable::heuristicTable[(t >> 32) & 0xFFFF];
        score += LookupTable::heuristicTable[(t >> 48) & 0xFFFF];

        // 3. Evaluate Squares (NEW)
        // There are 9 2x2 squares on a 4x4 board.
        // The shift positions correspond to the top-left cell of each square:
        // Row 0: 0, 4, 8
        // Row 1: 16, 20, 24
        // Row 2: 32, 36, 40
        constexpr int shifts[] = {0, 4, 8, 16, 20, 24, 32, 36, 40};

        for (const int s : shifts) {
            score += LookupTable::squareTable[getSquareIndex(board, s)];
        }

        return score;
    }

    Direction AISolver::findBestMove(const Board& board, const int depth) {
        const Bitboard currentBoard = board.getState().board;
        auto bestMove = Direction::Up;

        // Thinking time per move: 200ms
        // Long enough to think carefully, fast enough not to lag
        const auto startTime = std::chrono::high_resolution_clock::now();

        // Clear old cache to prevent memory overflow (optional)
        TranspositionTable::instance().clear();

        for (int dth = 1; dth <= depth; ++dth) {
            float currentBestScore = -std::numeric_limits<float>::max();
            auto currentBestMove = Direction::Up;
            bool foundMove = false;

            // Try 4 directions at the current depth
            for (constexpr Direction dirs[4] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right}; const auto dir : dirs) {
                Bitboard nextBoard = currentBoard;
                bool changed = false;

                // Logic giả lập di chuyển (Move Simulation)
                const bool needTranspose = (dir == Direction::Up || dir == Direction::Down);
                if (needTranspose) nextBoard = transpose64(nextBoard);
                Bitboard tempBoard = 0;
                for (int r = 0; r < 4; ++r) {
                    const Row row = (nextBoard >> (r * 16)) & Config::ROW_MASK;
                    Row newRow;
                    if (dir == Direction::Left || dir == Direction::Up)
                        newRow = LookupTable::moveLeftTable[row];
                    else
                        newRow = LookupTable::moveRightTable[row];
                    tempBoard |= (static_cast<Bitboard>(newRow) << (r * 16));
                }
                if (tempBoard != nextBoard) changed = true;
                nextBoard = tempBoard;
                if (needTranspose) nextBoard = transpose64(nextBoard);
                // ----------------------------------------

                if (changed) {
                    // Recursive call
                    if (const float score = expectimax(nextBoard, dth, false, 1.0f); score > currentBestScore) {
                        currentBestScore = score;
                        currentBestMove = dir;
                        foundMove = true;
                    }
                }
            }

            // Update the best result found at this depth
            if (foundMove) {
                bestMove = currentBestMove;
            }

            // Check time: If over 200ms, stop immediately and return the best available result
            auto currentTime = std::chrono::high_resolution_clock::now();
            const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
            if (constexpr long long timeLimitMs = 200; duration > timeLimitMs) {
                break;
            }
        }

        return bestMove;
    }

    float AISolver::expectimax(const Bitboard board, const int depth, const bool isPlayerTurn, const float cumulativeProb) {
        if (cumulativeProb < 0.0001f || depth == 0) {
            return evaluateBoard(board);
        }

        // CHANCE NODE: Only cache computer's turn (spawning tiles) because this state repeats most often
        if (!isPlayerTurn) {
            if (float cachedScore; TranspositionTable::instance().get(board, depth, cachedScore)) {  // Check if already computed
                return cachedScore;
            }
        }

        if (isPlayerTurn) {  // Max Node (Lượt người chơi)
            float maxVal = -std::numeric_limits<float>::max();
            bool canMove = false;

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
                    // Keep depth for Chance node
                    if (const float val = expectimax(nextBoard, depth, false, cumulativeProb); val > maxVal) maxVal = val;
                }
            }
            return canMove ? maxVal : 0;  // Heavy penalty if no moves are possible
        }

        // Chance Node (Computer's turn)
        float totalScore = 0;
        const int emptyCount = countEmpty(board);
        if (emptyCount == 0) return evaluateBoard(board);

        // We separate cumulativeProb from the cached value
        // The cached value must be the "pure average score" of the board state
        for (int i = 0; i < 16; ++i) {
            if (((board >> (i * 4)) & 0xF) == 0) {
                // Spawn tile 2 (0.9 probability)
                const Bitboard board2 = board | (static_cast<Bitboard>(1) << (i * 4));
                totalScore += 0.9f * expectimax(board2, depth - 1, true, cumulativeProb * 0.9f);

                // Spawn tile 4 (0.1 probability)
                const Bitboard board4 = board | (static_cast<Bitboard>(2) << (i * 4));
                totalScore += 0.1f * expectimax(board4, depth - 1, true, cumulativeProb * 0.1f);
            }
        }

        const float finalScore = totalScore / static_cast<float>(emptyCount);

        // Store in cache for reuse
        TranspositionTable::instance().put(board, depth, finalScore);

        return finalScore;
    }
}  // namespace tfe::core