#pragma once
#include "types.h"

namespace tfe::core {

    class LookupTable {
    public:
        static void init();

        // Input: Hàng hiện tại (16 bit). Output: Hàng mới sau khi di chuyển (16 bit).
        static Row moveLeftTable[65536];
        static Row moveRightTable[65536];

        // Điểm số nhận được khi thực hiện nước đi trên hàng đó
        static int scoreTable[65536];

        // Điểm đánh giá (Heuristic) của hàng đó (dùng cho AI đánh giá thế cờ)
        static float heuristicTable[65536];

    private:
        static void initRow(int row);
    };
}  // namespace tfe::core