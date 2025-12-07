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

    board.setTile(0, 0, 2);
    board.setTile(0, 1, 2);

    const bool moved = board.move(Direction::Left);

    EXPECT_TRUE(moved);
    EXPECT_EQ(board.getTile(0, 0), 4);
}

// Scenario: [4, 2, 2, 0] to Left -> [4, 4, ?, 0]
TEST(BoardTest, NoDoubleMerge) {
    Board board(4);
    clearBoard(board);

    board.setTile(0, 0, 4);
    board.setTile(0, 1, 2);
    board.setTile(0, 2, 2);

    board.move(Direction::Left);

    EXPECT_EQ(board.getTile(0, 0), 4);
    EXPECT_EQ(board.getTile(0, 1), 4);
}

TEST(BoardTest, GameOverCheck) {
    Board board(2);
    // 2 4
    // 4 2
    board.setTile(0, 0, 2);
    board.setTile(0, 1, 4);
    board.setTile(1, 0, 4);
    board.setTile(1, 1, 2);

    EXPECT_TRUE(board.isGameOver());

    // 2 2
    // 4 2
    board.setTile(0, 1, 2);
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
    board.setTile(2, 0, 2);
    board.setTile(3, 0, 2);

    const bool moved = board.move(Direction::Up);

    EXPECT_TRUE(moved);
    EXPECT_EQ(board.getTile(0, 0), 4);
    int count = 0;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            if (board.getTile(i, j) != 0) count++;

    EXPECT_EQ(count, 2);
}

// 2. Test merging a full row: [2, 2, 2, 2] -> [4, 4, 0, 0]
TEST(BoardTest, MergeFullRow) {
    Board board(4);
    clearBoard(board);

    // Row 0: [2, 2, 2, 2]
    for (int j = 0; j < 4; ++j) board.setTile(0, j, 2);

    board.move(Direction::Left);

    EXPECT_EQ(board.getTile(0, 0), 4);
    EXPECT_EQ(board.getTile(0, 1), 4);

    int count = 0;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            if (board.getTile(i, j) != 0) count++;

    EXPECT_EQ(count, 3);
}

// 3. Test moving over a gap: [2, 0, 2, 0] -> [4, 0, 0, 0]
TEST(BoardTest, MoveOverGap) {
    Board board(4);
    clearBoard(board);

    board.setTile(0, 0, 2);
    board.setTile(0, 2, 2);  // Gap in the middle

    board.move(Direction::Left);

    EXPECT_EQ(board.getTile(0, 0), 4);

    int count = 0;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            if (board.getTile(i, j) != 0) count++;

    EXPECT_EQ(count, 2);
}

// 4. Test no new tile spawns if no move is possible
TEST(BoardTest, NoMovePossible) {
    Board board(4);
    clearBoard(board);

    // [4, 0, 0, 0] -> Move Left -> No change
    board.setTile(0, 0, 4);

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
    board.setTile(0, 0, 1024);
    board.setTile(1, 1, 512);

    board.reset();

    int nonZero = 0;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            if (board.getTile(i, j) != 0) nonZero++;

    // After reset, the board should be clean and have only 2 initial numbers
    EXPECT_EQ(nonZero, 2);
    // The old (0,0) tile was 1024, now it's highly probable to be 0 or 2/4.
    // Ensure it's no longer 1024
    EXPECT_NE(board.getTile(0, 0), 1024);
}