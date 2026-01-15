#pragma once
#include "types.h"

namespace tfe::core {

    class LookupTable {
    public:
        static void init();

        static Row moveLeftTable[65536];
        static Row moveRightTable[65536];

        static int scoreTable[65536];

        static float heuristicTable[65536];

    private:
        static void initRow(int row);
    };
}  // namespace tfe::core