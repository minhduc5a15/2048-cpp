#pragma once
#include "types.h"

namespace tfe::core {

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
        static Row moveLeftTable[65536];
        static Row moveRightTable[65536];

        // Score received when performing a move on that row
        static int scoreTable[65536];

        // Heuristic score of that row (used for AI to evaluate board state)
        static float heuristicTable[65536];

    private:
        static void initRow(int row);
    };
}