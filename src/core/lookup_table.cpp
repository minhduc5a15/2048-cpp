#include "lookup_table.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

#include "config.h"

namespace tfe::core {

    // Define storage for static members
    Row LookupTable::moveLeftTable[TABLE_SIZE];
    Row LookupTable::moveRightTable[TABLE_SIZE];
    Bitboard LookupTable::colUpTable[TABLE_SIZE];
    Bitboard LookupTable::colDownTable[TABLE_SIZE];
    int LookupTable::scoreTable[TABLE_SIZE];
    int LookupTable::scoreRightTable[TABLE_SIZE];
    float LookupTable::heuristicTable[TABLE_SIZE];
    float LookupTable::squareTable[TABLE_SIZE];

    // --- Default Heuristic Weights ---
    // These values are used if no external weight file is loaded.
    // They are tuned to reward empty spaces, monotonic rows, and merging opportunities.
    static constexpr float SCORE_LOST_PENALTY = 200000.0f;
    static constexpr float SCORE_MONOTONICITY_POWER = 4.0f;
    static constexpr float SCORE_MONOTONICITY_WEIGHT = 47.0f;
    static constexpr float SCORE_SUM_POWER = 3.5f;
    static constexpr float SCORE_SUM_WEIGHT = 11.0f;
    static constexpr float SCORE_MERGES_WEIGHT = 700.0f;
    static constexpr float SCORE_EMPTY_WEIGHT = 270.0f;

    /**
     * @brief Spreads a 16-bit packed row (e.g., 0xABCD) into a 64-bit sparse column structure.
     * 
     * This is used for vertical moves. A column in the 64-bit board is spread across 
     * bits 0-3 (Row 0), 16-19 (Row 1), 32-35 (Row 2), and 48-51 (Row 3).
     * 
     * @param row The packed 16-bit representation of the column.
     * @return Bitboard with the tiles placed in Column 0 positions.
     */
    static Bitboard unpackColumn(const Row row) {
        Bitboard res = 0;
        res |= static_cast<Bitboard>(row & 0xF);                // Cell 0 -> Row 0 (bit 0)
        res |= static_cast<Bitboard>((row >> 4) & 0xF) << 16;   // Cell 1 -> Row 1 (bit 16)
        res |= static_cast<Bitboard>((row >> 8) & 0xF) << 32;   // Cell 2 -> Row 2 (bit 32)
        res |= static_cast<Bitboard>((row >> 12) & 0xF) << 48;  // Cell 3 -> Row 3 (bit 48)
        return res;
    }

    /**
     * @brief Reverses a 16-bit row (e.g., 0x1234 -> 0x4321).
     * Used to derive Right move tables from Left move logic.
     */
    static Row reverseRow(const Row row) { 
        return (row >> 12) | ((row >> 4) & 0x00F0) | ((row << 4) & 0x0F00) | (row << 12); 
    }

    void LookupTable::init() {
        // Initialize basic tables (Move Left, Score, Heuristics)
        for (int i = 0; i < TABLE_SIZE; ++i) {
            initRow(i);
        }

        // Post-processing: Derive other tables from the basic ones
        for (int i = 0; i < TABLE_SIZE; ++i) {
            // Move Right is just Move Left on a reversed row, then reversed back
            moveRightTable[i] = reverseRow(moveLeftTable[reverseRow(i)]);
            scoreRightTable[i] = scoreTable[reverseRow(i)];

            // Initialize Column Tables (for fast vertical moves without transposing)
            // "Up" on a column acts like "Left" on a row
            colUpTable[i] = unpackColumn(moveLeftTable[i]);
            
            // "Down" on a column acts like "Right" on a row
            colDownTable[i] = unpackColumn(moveRightTable[i]);
        }
    }

    bool LookupTable::loadWeights(const char* filepath) {
        // Force usage of default heuristics by ignoring the file.
        (void)filepath; // Prevent unused parameter warning
        return false;
    }

    // Helper: Unpacks 16-bit row to vector of 4 integers
    static std::vector<int> unpack(const int row) {
        std::vector<int> line(4);
        line[0] = (row >> 0) & 0xF;
        line[1] = (row >> 4) & 0xF;
        line[2] = (row >> 8) & 0xF;
        line[3] = (row >> 12) & 0xF;
        return line;
    }

    // Helper: Packs vector of 4 integers to 16-bit row
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

        // 1. Calculate Heuristic Score (Default Evaluation Function)
        float sum = 0;
        int empty = 0;
        int merges = 0;
        int prev = 0;
        int counter = 0;

        // Analyze row structure
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

        // Calculate Monotonicity (score is higher if values are strictly increasing/decreasing)
        float mono_left = 0, mono_right = 0;
        for (int i = 1; i < 4; ++i) {
            if (line[i - 1] > line[i])
                mono_left += std::pow(line[i - 1], SCORE_MONOTONICITY_POWER) - std::pow(line[i], SCORE_MONOTONICITY_POWER);
            else
                mono_right += std::pow(line[i], SCORE_MONOTONICITY_POWER) - std::pow(line[i - 1], SCORE_MONOTONICITY_POWER);
        }

        // Combine factors into final heuristic score
        heuristicTable[row] =
            SCORE_LOST_PENALTY + 
            SCORE_EMPTY_WEIGHT * empty + 
            SCORE_MERGES_WEIGHT * merges - 
            SCORE_MONOTONICITY_WEIGHT * std::min(mono_left, mono_right) - 
            SCORE_SUM_WEIGHT * sum;

        // 2. Calculate Move Left Result
        int score = 0;
        std::vector<int> temp;
        
        // Collect non-zero tiles
        for (int val : line)
            if (val != 0) temp.push_back(val);

        // Merge tiles
        if (!temp.empty()) {
            for (size_t i = 0; i < temp.size() - 1; ++i) {
                if (temp[i] == temp[i + 1]) {
                    temp[i]++; // Increment exponent (e.g. 2^1 -> 2^2)
                    score += (1 << temp[i]); // Add real value to score
                    temp.erase(temp.begin() + i + 1); // Remove merged tile
                }
            }
        }
        // Pad with zeros
        while (temp.size() < 4) temp.push_back(0);

        // Store results
        moveLeftTable[row] = pack(temp);
        scoreTable[row] = score;
    }
}  // namespace tfe::core