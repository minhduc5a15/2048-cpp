#include "ai_solver.h"

#include <chrono>
#include <limits>
#include <vector>

#include "lookup_table.h"
#include "transposition_table.h"

namespace tfe::core {

    /**
     * @brief Lightweight exception used to immediately unwind the recursion stack when time runs out.
     * 
     * While using exceptions for control flow is often discouraged, in this specific case of 
     * deep recursion with a hard real-time constraint, it provides a clean way to "abort" 
     * the search from any depth without passing error codes up every single stack frame.
     */
    struct TimeOutException {};

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

    static int countEmpty(const Bitboard board) {
        int count = 0;
        for (int i = 0; i < 16; ++i) {
            if (((board >> (i * 4)) & 0xF) == 0) count++;
        }
        return count;
    }

    /**
     * @brief Extracts a 2x2 square of tiles for the "Tuple Network" evaluation.
     * 
     * Used by the trained neural network weights (if loaded) to evaluate local patterns.
     */
    static inline uint16_t getSquareIndex(const Bitboard board, const int shift) {
        uint16_t idx = 0;
        idx |= (board >> shift) & 0xF;
        idx |= ((board >> (shift + 4)) & 0xF) << 4;
        idx |= ((board >> (shift + 16)) & 0xF) << 8;
        idx |= ((board >> (shift + 20)) & 0xF) << 12;
        return idx;
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

        // 3. Evaluate Squares (Local 2x2 patterns)
        // This captures spatial relationships that simple row/col checks miss.
        constexpr int shifts[] = {0, 4, 8, 16, 20, 24, 32, 36, 40};
        for (const int s : shifts) {
            score += LookupTable::squareTable[getSquareIndex(board, s)];
        }
        return score;
    }

    Direction AISolver::findBestMove(const Board& board, const int maxDepth) {
        const Bitboard currentBoard = board.getState().board;
        auto bestMove = Direction::Up;

        // Hard time limit: 150ms per move.
        // This ensures the game remains responsive.
        const auto startTime = std::chrono::high_resolution_clock::now();
        const auto deadline = startTime + std::chrono::milliseconds(150);

        TranspositionTable::instance().clear();
        int nodesVisited = 0;

        // --- Iterative Deepening ---
        // We search depth 1, then depth 2, then depth 3...
        // If we timeout at depth N, we fall back to the best move found at depth N-1.
        for (int depth = 1; depth <= maxDepth; ++depth) {
            float currentBestScore = -std::numeric_limits<float>::max();
            auto currentBestMove = Direction::Up;
            bool foundMove = false;

            try {
                // Try all 4 directions for the root node
                for (const auto dir : {Direction::Up, Direction::Down, Direction::Left, Direction::Right}) {
                    Bitboard nextBoard = 0;
                    bool changed = false;

                    // --- MOVE SIMULATION (Optimized with Lookup Tables) ---
                    if (dir == Direction::Left) {
                        for (int r = 0; r < 4; ++r) {
                            const Row row = (currentBoard >> (r * 16)) & 0xFFFF;
                            const Row newRow = LookupTable::moveLeftTable[row];
                            if (row != newRow) changed = true;
                            nextBoard |= (static_cast<Bitboard>(newRow) << (r * 16));
                        }
                    } else if (dir == Direction::Right) {
                        for (int r = 0; r < 4; ++r) {
                            const Row row = (currentBoard >> (r * 16)) & 0xFFFF;
                            const Row newRow = LookupTable::moveRightTable[row];
                            if (row != newRow) changed = true;
                            nextBoard |= (static_cast<Bitboard>(newRow) << (r * 16));
                        }
                    } else if (dir == Direction::Up) {
                        for (int c = 0; c < 4; ++c) {
                            const Row col = extractColumn(currentBoard, c);
                            const Bitboard newCol = LookupTable::colUpTable[col];
                            nextBoard |= (newCol << c);
                        }
                        if (nextBoard != currentBoard) changed = true;
                    } else {  // Down
                        for (int c = 0; c < 4; ++c) {
                            const Row col = extractColumn(currentBoard, c);
                            const Bitboard newCol = LookupTable::colDownTable[col];
                            nextBoard |= (newCol << c);
                        }
                        if (nextBoard != currentBoard) changed = true;
                    }

                    if (changed) {
                        // Start recursive search for this branch
                        float score = expectimax(nextBoard, depth, false, 1.0f, deadline, nodesVisited);
                        if (score > currentBestScore) {
                            currentBestScore = score;
                            currentBestMove = dir;
                            foundMove = true;
                        }
                    }
                }

                // If we completed this depth level without timeout, update the global best move.
                if (foundMove) {
                    bestMove = currentBestMove;
                }

            } catch (const TimeOutException&) {
                // Timeout occurred! Stop searching deeper.
                // We return the 'bestMove' from the previous fully completed depth.
                break;
            }
        }

        return bestMove;
    }

    float AISolver::expectimax(const Bitboard board, const int depth, const bool isPlayerTurn, const float cumulativeProb, std::chrono::high_resolution_clock::time_point deadline,
                               int& nodesVisited) {
        // --- TIME CHECK ---
        // Checking the system clock is expensive. We only check every 4096 nodes 
        // to minimize overhead while remaining responsive enough.
        if ((nodesVisited & 0xFFF) == 0) {
            if (std::chrono::high_resolution_clock::now() >= deadline) {
                throw TimeOutException();
            }
        }
        nodesVisited++;

        // Base cases:
        // 1. Probability is too low (Pruning): This branch is unlikely to happen.
        // 2. Depth limit reached.
        if (cumulativeProb < 0.0001f || depth == 0) {
            return evaluateBoard(board);
        }

        // Transposition Table Lookup (Memoization)
        // If we've seen this state before at a sufficient depth, return the cached score.
        // Note: We only cache Environment nodes (Chance nodes) usually, or we can cache both.
        // Here we cache when it's NOT player turn (i.e., before chance node evaluation).
        if (!isPlayerTurn) {
            float cachedScore;
            if (TranspositionTable::instance().get(board, depth, cachedScore)) return cachedScore;
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
                    float val = expectimax(next, depth, false, cumulativeProb, deadline, nodesVisited);
                    if (val > maxVal) maxVal = val;
                }
            }
            // RIGHT
            {
                Bitboard next = 0;
                for (int r = 0; r < 64; r += 16) next |= static_cast<Bitboard>(LookupTable::moveRightTable[(board >> r) & 0xFFFF]) << r;
                if (next != board) {
                    canMove = true;
                    float val = expectimax(next, depth, false, cumulativeProb, deadline, nodesVisited);
                    if (val > maxVal) maxVal = val;
                }
            }
            // UP
            {
                Bitboard next = 0;
                for (int c = 0; c < 4; ++c) next |= LookupTable::colUpTable[extractColumn(board, c)] << c;
                if (next != board) {
                    canMove = true;
                    float val = expectimax(next, depth, false, cumulativeProb, deadline, nodesVisited);
                    if (val > maxVal) maxVal = val;
                }
            }
            // DOWN
            {
                Bitboard next = 0;
                for (int c = 0; c < 4; ++c) next |= LookupTable::colDownTable[extractColumn(board, c)] << c;
                if (next != board) {
                    canMove = true;
                    float val = expectimax(next, depth, false, cumulativeProb, deadline, nodesVisited);
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
                totalScore += 0.9f * expectimax(board | (static_cast<Bitboard>(1) << (i * 4)), depth - 1, true, cumulativeProb * 0.9f, deadline, nodesVisited);
                // 10% chance of spawning 4 (Value 2)
                totalScore += 0.1f * expectimax(board | (static_cast<Bitboard>(2) << (i * 4)), depth - 1, true, cumulativeProb * 0.1f, deadline, nodesVisited);
            }
        }

        if (emptyCount == 0) return evaluateBoard(board);

        const float finalScore = totalScore / emptyCount;
        
        // Cache the result
        TranspositionTable::instance().put(board, depth, finalScore);

        return finalScore;
    }
}  // namespace tfe::core