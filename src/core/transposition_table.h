#pragma once
#include <unordered_map>

#include "types.h"

namespace tfe::core {

    struct TTEntry {
        int depth;
        float score;
    };

    class TranspositionTable {
    public:
        static TranspositionTable& instance();

        bool get(Bitboard board, int depth, float& score) const;

        void put(Bitboard board, int depth, float score);

        void clear();

    private:
        TranspositionTable() = default;
        std::unordered_map<Bitboard, TTEntry> table_;
    };
}  // namespace tfe::core