#pragma once

#include "core/types.h"
#include <optional>

namespace tfe::ai {

    /**
     * @brief Abstract base class for all AI agents.
     * 
     * This interface standardizes how an agent interacts with the game environment.
     * Both the Heuristic Search (Expectimax) and future RL agents must implement this.
     */
    class Agent {
    public:
        virtual ~Agent() = default;

        /**
         * @brief Determines the best move for a given board state.
         * 
         * @param board The current bitboard representation of the game.
         * @return The best direction to move, or std::nullopt if no move is possible/decided.
         */
        virtual std::optional<tfe::core::Direction> getBestMove(tfe::core::Bitboard board) = 0;
    };

} // namespace tfe::ai
