#include "bitboard_ops.h"

#include "config.h"
#include "lookup_table.h"

namespace tfe::core::BitboardOps {

    /**
     * @brief Transposes the 4x4 bitboard (swaps rows and columns).
     * * Visual Transformation:
     * A B C D      A E I M
     * E F G H  ->  B F J N
     * I J K L      C G K O
     * M N O P      D H L P
     *
     * Implementation uses a "Divide and Conquer" bitwise swap (SWAR):
     * 1. Swap 4x4 bit sub-blocks (Nibbles).
     * 2. Swap 16-bit rows (Top/Bottom halves).
     */
    Bitboard transpose64(const Bitboard b) {
        // --- Step 1: Swap 4-bit Nibbles (Exchange columns inside 16-bit rows) ---
        // 0xF0F0... keeps the diagonal nibbles that don't move relative to the swap.
        // 0x0000... selects nibbles that need to move 'Left' (Up in bit significance).
        // 0x0F0F... selects nibbles that need to move 'Right' (Down in bit significance).
        constexpr Bitboard MASK_NIBBLE_KEEP = 0xF0F00F0FF0F00F0FULL;
        constexpr Bitboard MASK_NIBBLE_SHIFT = 0x0000F0F00000F0F0ULL;  // Needs << 12
        constexpr Bitboard MASK_NIBBLE_BACK = 0x0F0F00000F0F0000ULL;   // Needs >> 12

        const Bitboard stage1 = (b & MASK_NIBBLE_KEEP) | ((b & MASK_NIBBLE_SHIFT) << 12) | ((b & MASK_NIBBLE_BACK) >> 12);

        // --- Step 2: Swap 16-bit Rows (Exchange top and bottom halves) ---
        // 0xFF00... keeps rows 0 and 2.
        // 0x00FF... selects parts of rows to move Down (>> 24).
        // 0x0000... selects parts of rows to move Up (<< 24).
        constexpr Bitboard MASK_ROW_KEEP = 0xFF00FF0000FF00FFULL;
        constexpr Bitboard MASK_ROW_DOWN = 0x00FF00FF00000000ULL;  // Needs >> 24
        constexpr Bitboard MASK_ROW_UP = 0x00000000FF00FF00ULL;    // Needs << 24

        return (stage1 & MASK_ROW_KEEP) | ((stage1 & MASK_ROW_DOWN) >> 24) | ((stage1 & MASK_ROW_UP) << 24);
    }

    int countEmpty(const Bitboard b) {
        int count = 0;
        for (int i = 0; i < 16; ++i) {
            if (((b >> (i * 4)) & 0xF) == 0) count++;
        }
        return count;
    }

    int countDistinctTiles(const Bitboard b) {
        uint16_t bitset = 0;
        for (int i = 0; i < 16; ++i) {
            const int val = (b >> (i * 4)) & 0xF;
            if (val > 0) bitset |= (1 << val);
        }
        int count = 0;
        while (bitset) {
            bitset &= (bitset - 1);
            count++;
        }
        return count;
    }

    std::pair<Bitboard, int> executeMove(Bitboard board, const Direction dir) {
        if (dir == Direction::Up || dir == Direction::Down) board = transpose64(board);

        Bitboard newBoard = 0;
        int moveScore = 0;

        for (int r = 0; r < 4; ++r) {
            const Row row = (board >> (r * 16)) & Config::ROW_MASK;
            Row newRow;

            if (dir == Direction::Left || dir == Direction::Up) {
                newRow = LookupTable::moveLeftTable[row];
            } else {
                newRow = LookupTable::moveRightTable[row];
            }

            moveScore += LookupTable::scoreTable[row];
            newBoard |= (static_cast<Bitboard>(newRow) << (r * 16));
        }

        if (dir == Direction::Up || dir == Direction::Down) newBoard = transpose64(newBoard);

        return {newBoard, moveScore};
    }

}  // namespace tfe::core::BitboardOps
