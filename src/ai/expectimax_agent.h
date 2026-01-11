#pragma once

#include "agent.h"
#include <unordered_map>

namespace tfe::ai {

    /**
     * @brief Entry for the Transposition Table.
     * Stores the result of a search to avoid re-computation.
     */
    struct TranspositionEntry {
        int depth;      // Remaining depth when this result was recorded
        float score;    // Calculated score
    };

    /**
     * @brief Transposition Table type definition.
     * Key: Bitboard (Game State), Value: Cached search result.
     */
    using TranspositionTable = std::unordered_map<tfe::core::Bitboard, TranspositionEntry>;

    /**
     * @brief Expectimax Search Agent.
     * 
     * Uses Iterative Deepening Depth-First Search (IDDFS) with Expectimax,
     * Transposition Tables, and Probability Pruning.
     * Based on the logic from nneonneo/2048-ai.
     */
    class ExpectimaxAgent : public Agent {
    public:
        ExpectimaxAgent();
        ~ExpectimaxAgent() override = default;

        /**
         * @brief Calculates the best move using Expectimax search.
         * Implements Iterative Deepening within a time budget.
         */
        std::optional<tfe::core::Direction> getBestMove(tfe::core::Bitboard board) override;

    private:
        // Configuration
        static constexpr float CPROB_THRESH_BASE = 0.0001f;
        static constexpr int CACHE_DEPTH_LIMIT = 15;
        static constexpr long TIME_BUDGET_MS = 200;
        static constexpr int MAX_DEPTH_CAP = 12;

        // Internal State
        TranspositionTable transTable_;
    };

} // namespace tfe::ai
