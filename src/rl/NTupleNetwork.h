#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace tfe::rl {

    /**
     * @brief N-Tuple Network for Reinforcement Learning (TD-Learning).
     * 
     * This class manages a collection of tuples (patterns of cells) and their associated weights.
     * It allows evaluating a board state by summing the weights of all active tuple patterns.
     */
    class NTupleNetwork {
    public:
        NTupleNetwork();
        ~NTupleNetwork() = default;

        /**
         * @brief Registers a new tuple (pattern) to the network.
         * 
         * @param cells A list of cell indices (0..15) that make up this tuple.
         *              Indices usually follow row-major order (0 is top-left, 3 is top-right).
         */
        void addTuple(const std::vector<int>& cells);

        /**
         * @brief Computes the lookup index for a specific tuple given a board state.
         * 
         * The index is formed by concatenating the values (4-bit nibbles) of the cells in the tuple.
         * 
         * @param board The current game board (64-bit bitboard).
         * @param tupleId The ID of the tuple (index in the internal tuples vector).
         * @return The computed index into the weight table for this tuple.
         */
        size_t computeIndex(uint64_t board, size_t tupleId) const;

        /**
         * @brief Evaluates the board state by summing the weights of all tuples.
         * 
         * @param board The current game board.
         * @return The estimated value of the state.
         */
        float evaluate(uint64_t board) const;

        /**
         * @brief Updates the weight for a specific feature.
         * 
         * @param tupleId The ID of the tuple.
         * @param index The index within the tuple's weight table.
         * @param delta The value to add to the weight.
         */
        void updateWeightsForIndex(size_t tupleId, size_t index, float delta);

        /**
         * @brief Saves the network weights to a binary file.
         * 
         * @param path The file path.
         */
        void save(const std::string& path) const;

        /**
         * @brief Loads the network weights from a binary file.
         * 
         * @param path The file path.
         */
        void load(const std::string& path);

    private:
        // Definition of tuples (which cells belong to which tuple)
        std::vector<std::vector<int>> tuples_;

        // Weights for each tuple. 
        // weights_[i] corresponds to tuples_[i].
        // Each inner vector size is 16^(tuple_size).
        std::vector<std::vector<float>> weights_;
    };

} // namespace tfe::rl
