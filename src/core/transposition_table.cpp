#include "transposition_table.h"

namespace tfe::core {

    TranspositionTable& TranspositionTable::instance() {
        static TranspositionTable instance;
        return instance;
    }

    bool TranspositionTable::get(const Bitboard board, const int depth, float& score) const {
        auto it = table_.find(board);
        if (it != table_.end()) {
            if (it->second.depth >= depth) {
                score = it->second.score;
                return true;
            }
        }
        return false;
    }

    void TranspositionTable::put(const Bitboard board, const int depth, const float score) { table_[board] = {depth, score}; }

    void TranspositionTable::clear() { table_.clear(); }
}  // namespace tfe::core