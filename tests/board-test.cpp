#include "core/board.h"

#include <gtest/gtest.h>

using namespace tfe::core;

void clearBoard(Board& board) {
    const int size = board.getSize();
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            board.setTile(i, j, 0);
        }
    }
}

TEST(BoardTest, Initialization) {
    const Board board(4);
    EXPECT_EQ(board.getSize(), 4);

    int nonZeroCount = 0;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (board.getTile(i, j) != 0) nonZeroCount++;
        }
    }
    EXPECT_EQ(nonZeroCount, 2);
}

// Scenario: [2, 2, 0, 0] to Left -> [4, ?, 0, 0]
TEST(BoardTest, MoveLeftMerge) {
    Board board(4);
    clearBoard(board);

    // 1 -> Exponent 1 (Value 2)
    board.setTile(0, 0, 1);
    board.setTile(0, 1, 1);

    const bool moved = board.move(Direction::Left);

    EXPECT_TRUE(moved);
    // 2 -> Exponent 2 (Value 4)
    EXPECT_EQ(board.getTile(0, 0), 2);
}

// Scenario: [4, 2, 2, 0] to Left -> [4, 4, ?, 0]
TEST(BoardTest, NoDoubleMerge) {
    Board board(4);
    clearBoard(board);

    // 2 -> Value 4, 1 -> Value 2
    board.setTile(0, 0, 2);
    board.setTile(0, 1, 1);
    board.setTile(0, 2, 1);

    board.move(Direction::Left);

    EXPECT_EQ(board.getTile(0, 0), 2); // 4
    EXPECT_EQ(board.getTile(0, 1), 2); // 4
}

TEST(BoardTest, GameOverCheck) {
    Board board(4);
    
    // Fill the board with a checkerboard pattern so no merges are possible
    // 2 4 2 4
    // 4 2 4 2
    // 2 4 2 4
    // 4 2 4 2
    for(int r = 0; r < 4; ++r) {
        for(int c = 0; c < 4; ++c) {
            // (r+c)%2 == 0 -> 2 (exp 1), else 4 (exp 2)
            board.setTile(r, c, ((r + c) % 2 == 0) ? 1 : 2);
        }
    }

    EXPECT_TRUE(board.isGameOver());

    // Make a move possible: change (0,1) to 2 (exp 1) so (0,0) and (0,1) can merge
    board.setTile(0, 1, 1);
    EXPECT_FALSE(board.isGameOver());
}

// 1. Test a different move direction (Up) to ensure correct axis rotation logic
TEST(BoardTest, MoveUpBasic) {
    Board board(4);
    clearBoard(board);

    // Setup:
    // ...
    // 2 ...
    // 2 ...
    board.setTile(2, 0, 1); // 2
    board.setTile(3, 0, 1); // 2

    const bool moved = board.move(Direction::Up);

    EXPECT_TRUE(moved);
    EXPECT_EQ(board.getTile(0, 0), 2); // 4
    int count = 0;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            if (board.getTile(i, j) != 0) count++;

    // 4 + random tile
    EXPECT_EQ(count, 2);
}

// 2. Test merging a full row: [2, 2, 2, 2] -> [4, 4, 0, 0]
TEST(BoardTest, MergeFullRow) {
    Board board(4);
    clearBoard(board);

    // Row 0: [2, 2, 2, 2]
    for (int j = 0; j < 4; ++j) board.setTile(0, j, 1);

    board.move(Direction::Left);

    EXPECT_EQ(board.getTile(0, 0), 2); // 4
    EXPECT_EQ(board.getTile(0, 1), 2); // 4

    int count = 0;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            if (board.getTile(i, j) != 0) count++;

    EXPECT_EQ(count, 3); // 2 merged + 1 spawned
}

// 3. Test moving over a gap: [2, 0, 2, 0] -> [4, 0, 0, 0]
TEST(BoardTest, MoveOverGap) {
    Board board(4);
    clearBoard(board);

    board.setTile(0, 0, 1); // 2
    board.setTile(0, 2, 1); // 2

    board.move(Direction::Left);

    EXPECT_EQ(board.getTile(0, 0), 2); // 4

    int count = 0;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            if (board.getTile(i, j) != 0) count++;

    EXPECT_EQ(count, 2); // 1 merged + 1 spawned
}

// 4. Test no new tile spawns if no move is possible
TEST(BoardTest, NoMovePossible) {
    Board board(4);
    clearBoard(board);

    // [4, 0, 0, 0] -> Move Left -> No change
    board.setTile(0, 0, 2); // 4

    // Count tiles before move
    int tilesBefore = 0;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            if (board.getTile(i, j) != 0) tilesBefore++;

    const bool moved = board.move(Direction::Left);

    EXPECT_FALSE(moved);

    int tilesAfter = 0;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            if (board.getTile(i, j) != 0) tilesAfter++;

    // Number of tiles must remain the same (no junk numbers spawned)
    EXPECT_EQ(tilesBefore, tilesAfter);
}

// 5. Test Reset feature
TEST(BoardTest, ResetBoard) {
    Board board(4);
    board.setTile(0, 0, 10); // 1024 (2^10)
    board.setTile(1, 1, 9);  // 512 (2^9)

    board.reset();

    int nonZero = 0;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            if (board.getTile(i, j) != 0) nonZero++;

    // After reset, the board should be clean and have only 2 initial numbers
    EXPECT_EQ(nonZero, 2);
    // The old (0,0) tile was 10 (1024), now it's highly probable to be 0 or 1/2.
    // Ensure it's no longer 10
    EXPECT_NE(board.getTile(0, 0), 10);
}
