#pragma once
#include <cstdint>
#include <vector>

#include "types.h"

namespace tfe::core {

    /**
     * @struct TTEntry
     * @brief Represents a single entry in the Transposition Table.
     * 
     * Designed to fit exactly into 16 bytes.
     * Most CPU cache lines are 64 bytes, so a single cache fetch retrieves
     * exactly 4 contiguous entries, improving memory access patterns.
     */
    struct TTEntry {
        uint64_t key;        // 8 bytes - The full 64-bit board state acts as the unique key.
        float score;         // 4 bytes - The cached evaluation score for this state.
        uint8_t depth;       // 1 byte  - The depth at which this score was calculated.
        uint8_t padding[3];  // 3 bytes - Padding to align the struct to 16 bytes.
    };

    /**
     * @class TranspositionTable
     * @brief A large hash table for caching AI search results (Memoization).
     *
     * In the Expectimax algorithm, many board states are reached via different
     * move sequences (transpositions). For example:
     * - Move Up -> Move Left
     * - Move Left -> Move Up (often results in same state)
     * 
     * By caching the evaluation score of a state, we can skip re-calculating
     * the entire subtree if we encounter the same state again at an equal or
     * greater depth.
     */
    class TranspositionTable {
    public:
        /**
         * @brief Singleton accessor.
         * @return Reference to the global Transposition Table instance.
         */
        static TranspositionTable& instance();

        /**
         * @brief Retrieves a cached score for a given board state.
         * 
         * @param board The board state (key).
         * @param depth The required depth. If the cached entry has less depth, it's invalid.
         * @param score [Output] The retrieved score.
         * @return True if a valid entry was found, False otherwise.
         */
        bool get(Bitboard board, int depth, float& score) const;

        /**
         * @brief Stores a computed score in the table.
         * 
         * @param board The board state (key).
         * @param depth The depth of the search that produced this score.
         * @param score The evaluated score.
         */
        void put(Bitboard board, int depth, float score);

        /**
         * @brief Clears the entire table.
         * Efficiently resets memory using memset.
         */
        void clear();

    private:
        TranspositionTable();

        // 2^22 = 4,194,304 entries.
        // Total memory: 4M * 16B = ~64MB.
        // Large enough to hold significant history, small enough to fit in RAM easily.
        static constexpr size_t TABLE_SIZE = 1 << 22;
        
        // Bitmask for fast modulo operations (TABLE_SIZE must be a power of 2).
        static constexpr size_t TABLE_MASK = TABLE_SIZE - 1;

        std::vector<TTEntry> table_;
    };
}  // namespace tfe::core