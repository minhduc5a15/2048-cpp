#pragma once
#include "board.h"

namespace tfe::core {

    class AISolver {
    public:
        // Trả về nước đi tốt nhất (0: Up, 1: Down, 2: Left, 3: Right)
        // depth: Độ sâu tìm kiếm (ví dụ 3-4 lớp)
        static Direction findBestMove(const Board& board, int depth = 4); // Mặc định depth 4 là khá mạnh

    private:
        // Đánh giá điểm số của bảng hiện tại (sử dụng LookupTable)
        static float evaluateBoard(Bitboard board);
        
        // Node Max (Lượt người chơi)
        static float expectimax(Bitboard board, int depth, bool isPlayerTurn, float cumulativeProb);
    };
}
