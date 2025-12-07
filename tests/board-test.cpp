#include <gtest/gtest.h>
#include "core/board.h"

using namespace tfe::core;

TEST(BoardTest, Initialization) {
    Board board(4);
    EXPECT_EQ(board.getSize(), 4);

    int nonZeroCount = 0;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (board.getTile(i, j) != 0) nonZeroCount++;
        }
    }
    EXPECT_EQ(nonZeroCount, 2);
}

// Test 2: Kiểm tra logic Gộp (Merge) cơ bản
// Kịch bản: [2, 2, 0, 0] sang Trái -> [4, ?, 0, 0] (? là số random mới sinh ra hoặc 0)
TEST(BoardTest, MoveLeftMerge) {
    Board board(4);

    // Setup tình huống giả định (Xóa sạch bàn cờ trước)
    // Lưu ý: Board không có hàm clearAll public, ta dùng setTile đè lên
    for(int i=0; i<4; ++i)
        for(int j=0; j<4; ++j)
            board.setTile(i, j, 0);

    board.setTile(0, 0, 2);
    board.setTile(0, 1, 2);

    // Thực hiện di chuyển
    bool moved = board.move(Direction::Left);

    EXPECT_TRUE(moved);
    EXPECT_EQ(board.getTile(0, 0), 4); // Hai số 2 gộp thành 4
}

// Test 3: Kiểm tra không gộp kép trong 1 lần move
// Kịch bản: [4, 2, 2, 0] sang Trái -> [4, 4, ?, 0] (Chứ không phải [8, ?, ?, ?])
TEST(BoardTest, NoDoubleMerge) {
    Board board(4);

    // Clear
    for(int i=0; i<4; ++i) for(int j=0; j<4; ++j) board.setTile(i, j, 0);

    board.setTile(0, 0, 4);
    board.setTile(0, 1, 2);
    board.setTile(0, 2, 2);

    board.move(Direction::Left);

    EXPECT_EQ(board.getTile(0, 0), 4);
    EXPECT_EQ(board.getTile(0, 1), 4); // Hai số 2 phía sau gộp thành 4
}

// Test 4: Game Over
TEST(BoardTest, GameOverCheck) {
    Board board(2);

    // Lấp đầy bàn cờ:
    // 2 4
    // 4 2
    // Không thể di chuyển
    board.setTile(0, 0, 2);
    board.setTile(0, 1, 4);
    board.setTile(1, 0, 4);
    board.setTile(1, 1, 2);

    EXPECT_TRUE(board.isGameOver());

    // Trường hợp còn đi được:
    // 2 2
    // 4 2
    board.setTile(0, 1, 2);
    EXPECT_FALSE(board.isGameOver());
}