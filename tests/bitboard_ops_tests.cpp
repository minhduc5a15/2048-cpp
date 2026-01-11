#include <gtest/gtest.h>
#include "core/bitboard_ops.h"
#include "core/lookup_table.h"

using namespace tfe::core;

class BitboardOpsTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        LookupTable::init();
    }
};

TEST_F(BitboardOpsTest, Transpose64) {
    // Row 0: 1 2 3 4 (nibbles)
    Bitboard b = 0x4321; 
    
    // Transposed:
    // Row 0: 1 0 0 0
    // Row 1: 2 0 0 0
    // Row 2: 3 0 0 0
    // Row 3: 4 0 0 0
    Bitboard expected = 0x0001 | (0x0002ULL << 16) | (0x0003ULL << 32) | (0x0004ULL << 48);
    
    EXPECT_EQ(BitboardOps::transpose64(b), expected);
    EXPECT_EQ(BitboardOps::transpose64(expected), b);
}

TEST_F(BitboardOpsTest, CountEmpty) {
    Bitboard b = 0; // All empty
    EXPECT_EQ(BitboardOps::countEmpty(b), 16);
    
    b = 0x123456789ABCDEF0ULL; // One zero at end (Col 3 of Row 3? No, nibble 0 is Col 0 Row 0. Nibble 15 is Col 3 Row 3)
    // 0x...0 means bits 0-3 are 0. So Row 0 Col 0 is empty.
    EXPECT_EQ(BitboardOps::countEmpty(b), 1);
    
    b = ~0ULL; // No zero
    EXPECT_EQ(BitboardOps::countEmpty(b), 0);
}

TEST_F(BitboardOpsTest, ExecuteMove) {
    // 2 2 0 0 -> Left -> 4 0 0 0, Reward 4
    // 2 is 2^1 = 1. 4 is 2^2 = 2.
    // Nibbles: 1 1 0 0 -> 0x0011 (Low bits are Col 0, then Col 1)
    Bitboard b = 0x0011; 
    auto [newB, reward] = BitboardOps::executeMove(b, Direction::Left);
    EXPECT_EQ(newB, 0x0002);
    EXPECT_EQ(reward, 4);
    
    // 2 2 4 4 -> Left -> 4 8 0 0, Reward 4+8=12
    // Nibbles: 1 1 2 2 -> 0x2211
    b = 0x2211;
    std::tie(newB, reward) = BitboardOps::executeMove(b, Direction::Left);
    // 4 8 0 0 -> 2 3 0 0 -> 0x0032
    EXPECT_EQ(newB, 0x0032);
    EXPECT_EQ(reward, 12);
}
