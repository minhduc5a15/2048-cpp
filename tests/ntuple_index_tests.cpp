#include <gtest/gtest.h>
#include "rl/NTupleNetwork.h"
#include "core/types.h"
#include <fstream>

using namespace tfe::rl;

class NTupleIndexTest : public ::testing::Test {
protected:
    NTupleNetwork network;
};

TEST_F(NTupleIndexTest, ComputeIndexExplicit) {
    // Construct a board with known values
    // Row 0: 1, 2, 3, 4 (Indices 0, 1, 2, 3)
    // Row 1: 5, 6, 7, 8 (Indices 4, 5, 6, 7)
    // Row 2: 9, 10, 11, 12
    // Row 3: 13, 14, 15, 0
    
    tfe::core::Bitboard board = 0;
    
    // Set Row 0
    board |= (uint64_t(1) << (0 * 4));
    board |= (uint64_t(2) << (1 * 4));
    board |= (uint64_t(3) << (2 * 4));
    board |= (uint64_t(4) << (3 * 4));
    
    // Set Row 1
    board |= (uint64_t(5) << (4 * 4));
    board |= (uint64_t(6) << (5 * 4));
    board |= (uint64_t(7) << (6 * 4));
    board |= (uint64_t(8) << (7 * 4));

    // Tuple 0 is Row 0 {0, 1, 2, 3}
    // Index = val(0)*16^0 + val(1)*16^1 + val(2)*16^2 + val(3)*16^3
    //       = 1*1 + 2*16 + 3*256 + 4*4096
    //       = 1 + 32 + 768 + 16384 = 17185
    size_t idx0 = network.computeIndex(board, 0);
    EXPECT_EQ(idx0, 17185);

    // Tuple 4 is Col 0 {0, 4, 8, 12}
    // Cells: 0 (val 1), 4 (val 5), 8 (val 9 - unset in my board code above, wait)
    // I only set Row 0 and Row 1 fully.
    // Row 2, Row 3 are 0.
    // So Cell 8 is 0, Cell 12 is 0.
    // Index = 1*1 + 5*16 + 0 + 0 = 1 + 80 = 81.
    size_t idx4 = network.computeIndex(board, 4);
    EXPECT_EQ(idx4, 81);
}

TEST_F(NTupleIndexTest, EvaluateSumsWeights) {
    tfe::core::Bitboard board = 0;
    // Set a simple board: Row 0 = {1, 0, 0, 0}
    board |= (uint64_t(1) << 0);

    // Tuple 0 (Row 0): {0, 1, 2, 3} -> values {1, 0, 0, 0} -> Index 1
    size_t idx0 = network.computeIndex(board, 0);
    EXPECT_EQ(idx0, 1);

    // Tuple 12 (Snake 1): {3, 2, 1, 0} -> values {0, 0, 0, 1} -> Index 1 * 16^3 = 4096
    size_t idx12 = network.computeIndex(board, 12);
    EXPECT_EQ(idx12, 4096);

    // Update weights
    network.updateWeightsForIndex(0, idx0, 10.5f);
    network.updateWeightsForIndex(12, idx12, 5.5f);

    // Evaluate
    // Should be sum of all tuples. Most are index 0 (all empty).
    // Let's assume initialized weights are 0.
    // But index 0 might be shared?
    // Tuple 1 (Row 1): {4,5,6,7} -> all 0 -> Index 0.
    // If we only update specific indices, others remain 0.
    // However, if multiple tuples map to index 0, and index 0 has weight 0, sum is still 0 + explicit weights.
    
    float val = network.evaluate(board);
    // Total = 10.5 + 5.5 + (other 12 tuples * weight[0])
    // weight[0] is 0.0f by default.
    EXPECT_FLOAT_EQ(val, 16.0f);
}

TEST_F(NTupleIndexTest, SaveLoad) {
    tfe::core::Bitboard board = 0x1234567890ABCDEF; 
    // Just some pattern
    
    // Modify some weights
    network.updateWeightsForIndex(0, 12345, 3.14f);
    network.updateWeightsForIndex(5, 54321, 2.71f);
    
    float valBefore = network.evaluate(board);
    
    std::string path = "test_weights.bin";
    network.save(path);
    
    NTupleNetwork loadedNet;
    // loadedNet initializes with default tuples. load() should overwrite.
    loadedNet.load(path);
    
    float valAfter = loadedNet.evaluate(board);
    EXPECT_EQ(valBefore, valAfter);
    
    // Verify specific weight persistence
    // We can't access weights directly but we can infer from evaluate if we craft board?
    // Or just trust evaluate sum on random board.
    
    // Let's check size/file match
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    size_t fileSize = in.tellg();
    // 14 tuples * (1 byte len + 4 bytes indices + 8 bytes count + 65536*4 bytes data) + header
    // Header: 9 ("NTUPLEv1\n") + 4 (numTuples) = 13
    // Per tuple: 1 + 4 + 8 + 262144 = 262157
    // Total approx: 13 + 14 * 262157 = 3,670,211 bytes.
    // 3.5MB.
    EXPECT_GT(fileSize, 3000000);
    
    in.close();
    std::remove(path.c_str());
}
