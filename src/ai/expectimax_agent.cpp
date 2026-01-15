#include "expectimax_agent.h"
#include "core/lookup_table.h"
#include "core/bitboard_ops.h"
#include "core/config.h"
#include <limits>
#include <algorithm>
#include <chrono>

namespace tfe::ai {

    using namespace tfe::core;

    // --- Heuristic Evaluation ---
    // Uses pre-computed tables for speed.
    static float evaluateBoard(const Bitboard board) {
        float score = 0;
        // Row scores
        for (int r = 0; r < 4; ++r) {
            score += LookupTable::heuristicTable[(board >> (r * 16)) & 0xFFFF];
        }
        // Column scores
        Bitboard t = BitboardOps::transpose64(board);
        for (int r = 0; r < 4; ++r) {
            score += LookupTable::heuristicTable[(t >> (r * 16)) & 0xFFFF];
        }
        return score;
    }

    // --- Search Context ---
    // Passed recursively to avoid global state.
    struct SearchState {
        TranspositionTable* transTable;
        float cprobThreshold;
    };

    // Forward Declaration
    static float scoreChanceNode(SearchState& state, Bitboard board, int depth, float cprob);

    // --- Max Node (Player Move) ---
    static float scoreMoveNode(SearchState& state, Bitboard board, int depth, float cprob) {
        // Base case: Depth limit reached
        if (depth == 0) {
            return evaluateBoard(board);
        }
        
        // Base case: Probability Pruning
        // If the probability of reaching this node is too low, use heuristic immediately.
        if (cprob < state.cprobThreshold) {
            return evaluateBoard(board);
        }

        // Transposition Table Lookup
        if (auto it = state.transTable->find(board); it != state.transTable->end()) {
            // Only use cached result if it was searched at least as deep as we want now.
            if (it->second.depth >= depth) {
                return it->second.score;
            }
        }

        float bestScore = -std::numeric_limits<float>::infinity();
        bool canMove = false;
        
        Direction dirs[] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};
        
        for (const auto dir : dirs) {
            auto [newBoard, _] = BitboardOps::executeMove(board, dir);
            
            if (newBoard != board) {
                canMove = true;
                // Move Node -> Chance Node (same depth convention)
                const float score = scoreChanceNode(state, newBoard, depth, cprob);
                if (score > bestScore) {
                    bestScore = score;
                }
            }
        }

        if (!canMove) {
            return 0; // Game Over (or huge penalty)
        }

        // Store result in Transposition Table
        (*state.transTable)[board] = {depth, bestScore};

        return bestScore;
    }

    // --- Chance Node (Random Spawn) ---
    static float scoreChanceNode(SearchState& state, const Bitboard board, const int depth, float cprob) {
        const int emptyCount = BitboardOps::countEmpty(board);
        if (emptyCount == 0) return 0;

        const float p_cell = cprob / emptyCount;
        
        // Pruning optimization
        if (p_cell < state.cprobThreshold) {
            return evaluateBoard(board); 
        }

        float totalScore = 0;
        
        for (int i = 0; i < 16; ++i) {
            if (((board >> (i * 4)) & 0xF) == 0) {
                
                // Spawn 2 (90% chance)
                // Recursive step: Chance -> Move (depth - 1)
                const Bitboard board2 = board | (static_cast<Bitboard>(1) << (i * 4));
                const float s2 = scoreMoveNode(state, board2, depth - 1, p_cell * Config::SPAWN_PROBABILITY_2);
                
                // Spawn 4 (10% chance)
                const Bitboard board4 = board | (static_cast<Bitboard>(2) << (i * 4));
                const float s4 = scoreMoveNode(state, board4, depth - 1, p_cell * (1.0f - Config::SPAWN_PROBABILITY_2));
                
                totalScore += Config::SPAWN_PROBABILITY_2 * s2 + (1.0f - Config::SPAWN_PROBABILITY_2) * s4;
            }
        }

        return totalScore / emptyCount;
    }

    // --- ExpectimaxAgent Implementation ---

    ExpectimaxAgent::ExpectimaxAgent() {
        // Reserve some space to avoid rehashes, though this is optional.
        transTable_.reserve(100000);
    }

    std::optional<Direction> ExpectimaxAgent::getBestMove(tfe::core::Bitboard board) {
        
        // 1. Dynamic Depth Calculation
        // Start depth is 3. Increase based on board complexity (distinct tiles).
        int distinctTiles = BitboardOps::countDistinctTiles(board);
        int targetDepth = std::max(3, distinctTiles - 2); 
        
        if (targetDepth > MAX_DEPTH_CAP) targetDepth = MAX_DEPTH_CAP;

        // 2. Setup Search Context
        auto startTime = std::chrono::high_resolution_clock::now();
        
        SearchState state;
        state.transTable = &transTable_;
        state.cprobThreshold = CPROB_THRESH_BASE;

        // Reset Transposition Table periodically to prevent stale entries/memory bloat?
        // For now, we keep it as is, or we could clear it if it gets too large.
        if (transTable_.size() > 500000) {
            transTable_.clear();
        }

        Direction bestMoveDir = Direction::Up; 
        bool foundAnyMove = false;

        Direction dirs[] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};
        
        // 3. Iterative Deepening Loop
        for (int depth = 1; depth <= targetDepth; ++depth) {
            
            float currentDepthBestScore = -std::numeric_limits<float>::infinity();
            auto currentDepthBestMove = Direction::Up;
            bool validMoveFoundAtThisDepth = false;

            for (const auto dir : dirs) {
                auto [newBoard, _] = BitboardOps::executeMove(board, dir);
                if (newBoard != board) {
                    
                    // Root always has probability 1.0
                    const float score = scoreChanceNode(state, newBoard, depth, 1.0f);
                    
                    if (score > currentDepthBestScore) {
                        currentDepthBestScore = score;
                        currentDepthBestMove = dir;
                    }
                    validMoveFoundAtThisDepth = true;
                }
            }

            if (validMoveFoundAtThisDepth) {
                bestMoveDir = currentDepthBestMove;
                foundAnyMove = true;
            }

            // Time Budget Check
            auto now = std::chrono::high_resolution_clock::now();
            const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
            if (duration > TIME_BUDGET_MS) {
                break; 
            }
        }

        if (!foundAnyMove) return std::nullopt;
        return bestMoveDir;
    }

} // namespace tfe::ai