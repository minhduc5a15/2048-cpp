#pragma once
#include "types.h"

namespace tfe::core {
    static constexpr int TABLE_SIZE = 1 << 16;  // 2^16 = 65536

    class LookupTable {
    public:
        /**
         * @brief Initializes the lookup tables.
         */
        static void init();

        /**
         * @brief Loads weights from a binary file.
         * @param filepath Path to the binary file containing weights.
         * @return True if loading was successful, false otherwise.
         */
        static bool loadWeights(const char* filepath);

        // Input: Current row (16 bits). Output: New row after moving (16 bits).
        static Row moveLeftTable[TABLE_SIZE];
        static Row moveRightTable[TABLE_SIZE];

        // Score received when performing a move on that row
        static int scoreTable[TABLE_SIZE];

        // Heuristic score of that row (used for AI to evaluate board state)
        static float heuristicTable[TABLE_SIZE];
        static float squareTable[TABLE_SIZE];

    private:
        static void initRow(int row);
    };
}  // namespace tfe::core