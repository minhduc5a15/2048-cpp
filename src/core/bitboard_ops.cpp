#include "bitboard_ops.h"
#include "lookup_table.h"
#include "config.h"

namespace tfe::core::BitboardOps {

    Bitboard transpose64(Bitboard x) {
        Bitboard a1 = x & 0xF0F00F0FF0F00F0FULL;
        Bitboard a2 = x & 0x0000F0F00000F0F0ULL;
        Bitboard a3 = x & 0x0F0F00000F0F0000ULL;
        Bitboard a = a1 | (a2 << 12) | (a3 >> 12);
        Bitboard b1 = a & 0xFF00FF0000FF00FFULL;
        Bitboard b2 = a & 0x00FF00FF00000000ULL;
        Bitboard b3 = a & 0x00000000FF00FF00ULL;
        return b1 | (b2 >> 24) | (b3 << 24);
    }

    int countEmpty(Bitboard b) {
        int count = 0;
        for (int i = 0; i < 16; ++i) {
            if (((b >> (i * 4)) & 0xF) == 0) count++;
        }
        return count;
    }

    int countDistinctTiles(Bitboard b) {
        uint16_t bitset = 0;
        for (int i = 0; i < 16; ++i) {
            int val = (b >> (i * 4)) & 0xF;
            if (val > 0) bitset |= (1 << val);
        }
        int count = 0;
        while (bitset) {
            bitset &= (bitset - 1);
            count++;
        }
        return count;
    }

    std::pair<Bitboard, int> executeMove(Bitboard board, Direction dir) {
        if (dir == Direction::Up || dir == Direction::Down) board = transpose64(board);

        Bitboard newBoard = 0;
        int moveScore = 0;

        for (int r = 0; r < 4; ++r) {
            Row row = (board >> (r * 16)) & Config::ROW_MASK;
            Row newRow;

            if (dir == Direction::Left || dir == Direction::Up)
                newRow = LookupTable::moveLeftTable[row];
            else
                newRow = LookupTable::moveRightTable[row];

            moveScore += LookupTable::scoreTable[row];
            newBoard |= (static_cast<Bitboard>(newRow) << (r * 16));
        }

        if (dir == Direction::Up || dir == Direction::Down) newBoard = transpose64(newBoard);

        return {newBoard, moveScore};
    }

}
