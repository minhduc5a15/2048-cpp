#include "ai_solver.h"
#include "lookup_table.h"
#include "config.h"
#include <algorithm>
#include <limits>

namespace tfe::core {

    // Helper: Transpose (copy từ board.cpp sang hoặc tách ra utility)
    static inline Bitboard transpose64(Bitboard x) {
        Bitboard a1 = x & 0xF0F00F0FF0F00F0FULL;
        Bitboard a2 = x & 0x0000F0F00000F0F0ULL;
        Bitboard a3 = x & 0x0F0F00000F0F0000ULL;
        Bitboard a = a1 | (a2 << 12) | (a3 >> 12);
        Bitboard b1 = a & 0xFF00FF0000FF00FFULL;
        Bitboard b2 = a & 0x00FF00FF00000000ULL;
        Bitboard b3 = a & 0x00000000FF00FF00ULL;
        return b1 | (b2 >> 24) | (b3 << 24);
    }

    // Helper: Đếm số ô trống (để tối ưu xác suất)
    static int countEmpty(Bitboard board) {
        int count = 0;
        for (int i = 0; i < 16; ++i) {
            if (((board >> (i * 4)) & 0xF) == 0) count++;
        }
        return count;
    }

    // Đánh giá bàn cờ dựa trên LookupTable (đã được train)
    float AISolver::evaluateBoard(Bitboard board) {
        // Tra cứu 4 hàng ngang
        float score = 
            LookupTable::heuristicTable[(board >> 0) & 0xFFFF] +
            LookupTable::heuristicTable[(board >> 16) & 0xFFFF] +
            LookupTable::heuristicTable[(board >> 32) & 0xFFFF] +
            LookupTable::heuristicTable[(board >> 48) & 0xFFFF];

        // Tra cứu 4 cột dọc (Transpose)
        Bitboard t = transpose64(board);
        score += 
            LookupTable::heuristicTable[(t >> 0) & 0xFFFF] +
            LookupTable::heuristicTable[(t >> 16) & 0xFFFF] +
            LookupTable::heuristicTable[(t >> 32) & 0xFFFF] +
            LookupTable::heuristicTable[(t >> 48) & 0xFFFF];

        return score;
    }

    Direction AISolver::findBestMove(const Board& boardData, int depth) {
        Bitboard currentBoard = boardData.getState().board;
        float bestScore = -std::numeric_limits<float>::max();
        int bestMove = -1;

        // Thử 4 hướng: Up, Down, Left, Right
        // Thứ tự trong Enum: Up=0, Down=1, Left=2, Right=3
        // Tuy nhiên trong Board::move logic:
        // Left/Right: Xử lý trực tiếp
        // Up/Down: Transpose -> Xử lý -> Transpose
        
        // Để đơn giản, ta tái hiện logic move của Board ở đây để không thay đổi state gốc
        // Hoặc ta có thể copy Board, nhưng copy Board hơi nặng.
        // Ta dùng bitboard thuần túy cho nhanh.

        // Mảng map Direction
        Direction dirs[4] = {Direction::Up, Direction::Down, Direction::Left, Direction::Right};

        for (int i = 0; i < 4; ++i) {
            Bitboard nextBoard = currentBoard;
            // int moveReward = 0;
            bool changed = false;

            // --- Logic Move Simulation (Copy từ Board::move) ---
            bool needTranspose = (dirs[i] == Direction::Up || dirs[i] == Direction::Down);
            if (needTranspose) nextBoard = transpose64(nextBoard);

            Bitboard tempBoard = 0;
            for (int r = 0; r < 4; ++r) {
                Row row = (nextBoard >> (r * 16)) & Config::ROW_MASK;
                Row newRow;
                // Left hoặc Up (đã transpose thành Left) dùng moveLeftTable
                if (dirs[i] == Direction::Left || dirs[i] == Direction::Up) 
                    newRow = LookupTable::moveLeftTable[row];
                else 
                    newRow = LookupTable::moveRightTable[row];
                
                tempBoard |= (static_cast<Bitboard>(newRow) << (r * 16));
                // moveReward += LookupTable::scoreTable[row]; // (Tùy chọn: cộng điểm reward thực tế)
            }
            if (tempBoard != nextBoard) changed = true;
            nextBoard = tempBoard;
            if (needTranspose) nextBoard = transpose64(nextBoard);
            // ---------------------------------------------------

            if (changed) {
                // Sang lượt máy tính (sinh số ngẫu nhiên) -> Expectation Node
                float score = expectimax(nextBoard, depth - 1, false, 1.0f);
                if (score > bestScore) {
                    bestScore = score;
                    bestMove = i;
                }
            }
        }

        if (bestMove != -1) return dirs[bestMove];
        return Direction::Up; // Fallback
    }

    float AISolver::expectimax(Bitboard board, int depth, bool isPlayerTurn, float cumulativeProb) {
        // Cắt nhánh nếu xác suất quá nhỏ (Pruning) để tăng tốc
        if (cumulativeProb < 0.0001f || depth == 0) {
            return evaluateBoard(board);
        }

        if (isPlayerTurn) { // Max Node (Người chơi)
            float maxVal = -std::numeric_limits<float>::max();
            bool canMove = false;

            // Thử 4 hướng
            for (int dir = 0; dir < 4; ++dir) {
                Bitboard nextBoard = board;
                bool needTranspose = (dir == 0 || dir == 1); // Up/Down
                if (needTranspose) nextBoard = transpose64(nextBoard);

                Bitboard tempBoard = 0;
                for (int r = 0; r < 4; ++r) {
                    Row row = (nextBoard >> (r * 16)) & 0xFFFF;
                    Row newRow;
                    if (dir == 0 || dir == 2) newRow = LookupTable::moveLeftTable[row];
                    else newRow = LookupTable::moveRightTable[row];
                    tempBoard |= (static_cast<Bitboard>(newRow) << (r * 16));
                }
                
                bool changed = (tempBoard != nextBoard);
                nextBoard = tempBoard;
                if (needTranspose) nextBoard = transpose64(nextBoard);

                if (changed) {
                    canMove = true;
                    float val = expectimax(nextBoard, depth - 1, false, cumulativeProb);
                    if (val > maxVal) maxVal = val;
                }
            }
            // Nếu không đi được nước nào -> Game Over -> Phạt nặng
            return canMove ? maxVal : 0; 
        } 
        else { // Chance Node (Máy tính sinh số)
            float avgScore = 0;
            int emptyCount = countEmpty(board);
            if (emptyCount == 0) return evaluateBoard(board); // Hết chỗ sinh -> Game Over (về lý thuyết)

            float prob2 = 0.9f * cumulativeProb / emptyCount; // Xác suất sinh số 2
            float prob4 = 0.1f * cumulativeProb / emptyCount; // Xác suất sinh số 4

            // Duyệt qua tất cả ô trống
            for (int i = 0; i < 16; ++i) {
                if (((board >> (i * 4)) & 0xF) == 0) {
                    // Trường hợp sinh ô 2 (mũ 1)
                    Bitboard board2 = board | (static_cast<Bitboard>(1) << (i * 4));
                    avgScore += 0.9f * expectimax(board2, depth - 1, true, prob2);

                    // Trường hợp sinh ô 4 (mũ 2)
                    Bitboard board4 = board | (static_cast<Bitboard>(2) << (i * 4));
                    avgScore += 0.1f * expectimax(board4, depth - 1, true, prob4);
                }
            }
            return avgScore; 
        }
    }
}
