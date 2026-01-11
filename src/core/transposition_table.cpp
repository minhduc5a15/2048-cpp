#include "transposition_table.h"

#include <cstring>

namespace tfe::core {

    TranspositionTable& TranspositionTable::instance() {
        static TranspositionTable instance;
        return instance;
    }

    TranspositionTable::TranspositionTable() {
        // Pre-allocate the entire table to avoid reallocations during runtime.
        table_.resize(TABLE_SIZE);
        clear();
    }

    bool TranspositionTable::get(Bitboard board, int depth, float& score) const {
        // Simple hash function: Use the board state itself as the hash.
        // Since TABLE_SIZE is a power of 2, we can use a bitmask (TABLE_MASK)
        // instead of the modulo operator (%) for speed.
        uint64_t key = board;
        key ^= key >> 33;
        key *= 0xff51afd7ed558ccdULL;
        key ^= key >> 33;
        key *= 0xc4ceb9fe1a85ec53ULL;
        key ^= key >> 33;
        const size_t index = key & TABLE_MASK;
        const TTEntry& entry = table_[index];

        // Check for Key Match (Collision check) AND Sufficient Depth
        // We only use the cached value if the stored search was at least as deep
        // as the current request. Shallow searches are not precise enough for deep recursion.
        if (entry.key == board && entry.depth >= depth) {
            score = entry.score;
            return true;
        }
        return false;
    }

    void TranspositionTable::put(Bitboard board, int depth, float score) {
        uint64_t key = board;
        key ^= key >> 33;
        key *= 0xff51afd7ed558ccdULL;
        key ^= key >> 33;
        key *= 0xc4ceb9fe1a85ec53ULL;
        key ^= key >> 33;
        const size_t index = key & TABLE_MASK;
        
        // Replacement Strategy: Always Replace
        // We overwrite the existing entry at this hash index.
        // While more complex strategies exist (e.g., replace if deeper, or separate buckets),
        // "Always Replace" is fast and works well for this game because recent nodes
        // are often more relevant to the current search path.
        table_[index] = TTEntry{board, score, static_cast<uint8_t>(depth), {0, 0, 0}};
    }

    void TranspositionTable::clear() {
        // memset is the fastest way to zero out a large block of memory.
        std::memset(table_.data(), 0, table_.size() * sizeof(TTEntry));
    }

}  // namespace tfe::core