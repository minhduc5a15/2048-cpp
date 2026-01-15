#include "lookup_table.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace tfe::core {

    constexpr int ROW_TABLE_SIZE = 65536;

    Row LookupTable::moveLeftTable[ROW_TABLE_SIZE];
    Row LookupTable::moveRightTable[ROW_TABLE_SIZE];
    int LookupTable::scoreTable[ROW_TABLE_SIZE];
    float LookupTable::heuristicTable[ROW_TABLE_SIZE];

    // Heuristic weights (referenced from nneonneo/2048-ai)
    // In the future, we can use Reinforcement Learning (RL) to fine-tune these values.
    static constexpr float SCORE_LOST_PENALTY = 200000.0f;
    static constexpr float SCORE_MONOTONICITY_POWER = 4.0f;
    static constexpr float SCORE_MONOTONICITY_WEIGHT = 47.0f;
    static constexpr float SCORE_SUM_POWER = 3.5f;
    static constexpr float SCORE_SUM_WEIGHT = 11.0f;
    static constexpr float SCORE_MERGES_WEIGHT = 700.0f;
    static constexpr float SCORE_EMPTY_WEIGHT = 270.0f;

    static Row reverseRow(const Row row) { return (row >> 12) | ((row >> 4) & 0x00F0) | ((row << 4) & 0x0F00) | (row << 12); }

    void LookupTable::init() {
        for (int i = 0; i < ROW_TABLE_SIZE; ++i) {
            initRow(i);
        }
        // Second pass for moveRightTable to ensure moveLeftTable is fully populated
        for (int i = 0; i < ROW_TABLE_SIZE; ++i) {
            moveRightTable[i] = reverseRow(moveLeftTable[reverseRow(i)]);
        }
    }

    static std::vector<int> unpack(const int row) {
        std::vector<int> line(4);
        line[0] = (row >> 0) & 0xF;
        line[1] = (row >> 4) & 0xF;
        line[2] = (row >> 8) & 0xF;
        line[3] = (row >> 12) & 0xF;
        return line;
    }

    static Row pack(const std::vector<int>& line) {
        Row row = 0;
        row |= (line[0] << 0);
        row |= (line[1] << 4);
        row |= (line[2] << 8);
        row |= (line[3] << 12);
        return row;
    }

    void LookupTable::initRow(const int row) {
        const auto line = unpack(row);

        // 1. Calculate Heuristic Score (Evaluate the quality of this row)
        float sum = 0;
        int empty = 0;
        int merges = 0;
        int prev = 0;
        int counter = 0;

        for (const int val : line) {
            sum += std::pow(val, SCORE_SUM_POWER);
            if (val == 0) {
                empty++;
            } else {
                if (prev == val)
                    counter++;
                else if (counter > 0) {
                    merges += 1 + counter;
                    counter = 0;
                }
                prev = val;
            }
        }
        if (counter > 0) merges += 1 + counter;

        float mono_left = 0, mono_right = 0;
        for (int i = 1; i < 4; ++i) {
            if (line[i - 1] > line[i])
                mono_left += std::pow(line[i - 1], SCORE_MONOTONICITY_POWER) - std::pow(line[i], SCORE_MONOTONICITY_POWER);
            else
                mono_right += std::pow(line[i], SCORE_MONOTONICITY_POWER) - std::pow(line[i - 1], SCORE_MONOTONICITY_POWER);
        }

        heuristicTable[row] = SCORE_LOST_PENALTY + SCORE_EMPTY_WEIGHT * empty + SCORE_MERGES_WEIGHT * merges -
                              SCORE_MONOTONICITY_WEIGHT * std::min(mono_left, mono_right) - SCORE_SUM_WEIGHT * sum;

        // 2. Calculate Move Left Logic
        int score = 0;
        std::vector<int> temp;
        for (int val : line)
            if (val != 0) temp.push_back(val);  // Compress

        if (!temp.empty()) {
            for (size_t i = 0; i < temp.size() - 1; ++i) {
                if (temp[i] == temp[i + 1]) {  // Merge
                    temp[i]++;
                    score += (1 << temp[i]);  // Add actual score (2^k)
                    temp.erase(temp.begin() + i + 1);
                }
            }
        }
        while (temp.size() < 4) temp.push_back(0);  // Pad with 0

        moveLeftTable[row] = pack(temp);
        scoreTable[row] = score;
    }
}  // namespace tfe::core