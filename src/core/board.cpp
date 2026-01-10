#include "board.h"

#include <algorithm>

#include "config.h"
#include "lookup_table.h"
#include "score/score-manager.h"
#include "utils/random-generator.h"

namespace tfe::core {

    /**
     * @brief Transposes a 4x4 bitboard (swaps rows and columns).
     * 
     * This function uses bitwise operations to perform a matrix transpose on the 64-bit integer
     * representation of the board. This allows vertical moves (Up/Down) to be processed
     * using the same logic as horizontal moves (Left/Right) by simply transposing, moving,
     * and transposing back.
     * 
     * Algorithm source: "Hacker's Delight", Chapter 7 (Transposing a Bit Matrix).
     */
    static inline Bitboard transpose64(const Bitboard x) {
        const Bitboard a1 = x & 0xF0F00F0FF0F00F0FULL;
        const Bitboard a2 = x & 0x0000F0F00000F0F0ULL;
        const Bitboard a3 = x & 0x0F0F00000F0F0000ULL;
        const Bitboard a = a1 | (a2 << 12) | (a3 >> 12);
        const Bitboard b1 = a & 0xFF00FF0000FF00FFULL;
        const Bitboard b2 = a & 0x00FF00FF00000000ULL;
        const Bitboard b3 = a & 0x00000000FF00FF00ULL;
        return b1 | (b2 >> 24) | (b3 << 24);
    }

    Board::Board(int) {
        // Initialize LookupTables once. 
        // These tables cache the results of all possible row moves (2^16 possibilities).
        static bool tableInitialized = false;
        if (!tableInitialized) {
            LookupTable::init();

            // Attempt to load pre-trained weights for the AI evaluation function.
            // This allows the AI to play significantly better than with a generic heuristic.
            LookupTable::loadWeights("./build/bin/tuple_weights.bin");

            tableInitialized = true;
        }
        
        // Load the persistent high score.
        highScore_ = tfe::score::ScoreManager::load_high_score();
        
        // Start a new game.
        reset();
    }

    void Board::reset() {
        board_ = 0;
        score_ = 0;
        hasReachedWinTile_ = false;
        notifyGameReset();
        
        // Standard 2048 start: 2 tiles.
        spawnRandomTile();
        spawnRandomTile();
    }

    Grid Board::getGrid() const {
        Grid result(4, std::vector<int>(4));
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                const Tile t = getTile(r, c);
                // Convert internal exponent storage (1, 2, 3...) back to game values (2, 4, 8...).
                result[r][c] = (t == 0) ? 0 : (1 << t);  
            }
        }
        return result;
    }

    Tile Board::getTile(const int row, const int col) const { 
        // Extract 4 bits corresponding to the tile at (row, col).
        // Each row is 16 bits. Each tile is 4 bits.
        return (board_ >> ((row * 16) + (col * 4))) & 0xF; 
    }

    void Board::setTile(const int row, const int col, const Tile value) {
        const int shift = (row * 16) + (col * 4);
        // Clear the existing 4 bits
        board_ &= ~(static_cast<Bitboard>(0xF) << shift);
        // Set the new value
        board_ |= (static_cast<Bitboard>(value) << shift);
    }

    void Board::transpose() { board_ = transpose64(board_); }

    /**
     * @brief Generates detailed move events for the GUI animation system.
     * 
     * The core Move logic uses efficient bitboard lookup tables which give the *result*
     * of a move instantly but do not tell us *which* specific tile moved where.
     * 
     * This function manually re-simulates the logic for a single row to calculate
     * exact source and destination coordinates for animations.
     * 
     * @param row The packed 16-bit row data before the move.
     * @param rIdx The index of the row currently being processed.
     * @param isTransposed Whether the board is currently transposed (vertical move).
     * @param isReverse Whether the move direction is reversed (Right/Down).
     * @param board Reference to the board to fire notifications.
     */
    static void generateMoveEvents(const Row row, const int rIdx, const bool isTransposed, const bool isReverse, const Board& board) {
        int cells[4];
        // Unpack row into individual cells
        for (int i = 0; i < 4; ++i) cells[i] = (row >> (i * 4)) & 0xF;

        // If moving Right or Down, we reverse the array to treat it as a standard "Move Left" operation,
        // then map the coordinates back to their original positions later.
        if (isReverse) {
            std::ranges::reverse(cells);
        }

        // Standard Move Left Simulation
        int targetPos = 0;
        bool merged[4] = {false, false, false, false};  // Track merges at target positions
        int resultValues[4] = {0, 0, 0, 0}; // Track values at target positions

        for (int sourcePos = 0; sourcePos < 4; ++sourcePos) {
            const int val = cells[sourcePos];
            if (val == 0) continue; // Skip empty cells

            // Check merge with previous tile
            if (targetPos > 0 && resultValues[targetPos - 1] == val && !merged[targetPos - 1]) {
                // --- MERGE CASE ---
                merged[targetPos - 1] = true;
                resultValues[targetPos - 1]++;                        // Increment exponent
                const int newVal = 1 << resultValues[targetPos - 1];  // Real value for display

                // Calculate real coordinates (handling reverse and transpose)
                const int mappedSrc = isReverse ? (3 - sourcePos) : sourcePos;
                const int mappedDst = isReverse ? (3 - (targetPos - 1)) : (targetPos - 1);

                int srcR, srcC, dstR, dstC;
                if (isTransposed) {
                    srcR = mappedSrc; srcC = rIdx;
                    dstR = mappedDst; dstC = rIdx;
                } else {
                    srcR = rIdx; srcC = mappedSrc;
                    dstR = rIdx; dstC = mappedDst;
                }

                // Notify UI: Tile moves from src to dst
                const int movingVal = 1 << val;
                board.notifyTileMove(srcR, srcC, dstR, dstC, movingVal);

                // Notify UI: Merge happens at dst
                board.notifyTileMerge(dstR, dstC, newVal);

            } else {
                // --- MOVE CASE (No Merge) ---
                resultValues[targetPos] = val;

                const int mappedSrc = isReverse ? (3 - sourcePos) : sourcePos;
                const int mappedDst = isReverse ? (3 - targetPos) : targetPos;

                int srcR, srcC, dstR, dstC;
                if (isTransposed) {
                    srcR = mappedSrc; srcC = rIdx;
                    dstR = mappedDst; dstC = rIdx;
                } else {
                    srcR = rIdx; srcC = mappedSrc;
                    dstR = rIdx; dstC = mappedDst;
                }

                // Only fire event if the tile actually changed position
                if (mappedSrc != mappedDst) {
                    const int movingVal = 1 << val;
                    board.notifyTileMove(srcR, srcC, dstR, dstC, movingVal);
                }

                targetPos++;
            }
        }
    }

    bool Board::move(const Direction dir) {
        // Strategy: 
        // 1. If moving Up/Down, Transpose the board so columns become rows.
        // 2. Treat Up as Left, Down as Right.
        // 3. Process each row using LookupTables.
        // 4. If moving Up/Down, Transpose back.

        const bool isTransposed = (dir == Direction::Up || dir == Direction::Down);
        const bool isReverse = (dir == Direction::Right || dir == Direction::Down);

        // Normalize to Left/Right logic by rotating if necessary
        if (isTransposed) transpose();

        Bitboard newBoard = 0;
        int moveScore = 0;

        // Process all 4 rows
        for (int r = 0; r < 4; ++r) {
            // Extract row r
            const Row row = (board_ >> (r * 16)) & Config::ROW_MASK;
            Row newRow;

            // Use lookup tables for O(1) move logic per row
            if (dir == Direction::Left || dir == Direction::Up) {
                newRow = LookupTable::moveLeftTable[row];
                moveScore += LookupTable::scoreTable[row];
            } else {
                newRow = LookupTable::moveRightTable[row];
                moveScore += LookupTable::scoreRightTable[row];
            }

            // Pack the new row back into the new board state
            newBoard |= (static_cast<Bitboard>(newRow) << (r * 16));

            // Generate Animation Events for the UI
            // We check (row != newRow || true) to ensure events are generated.
            // Even if the row structure looks similar, the UI might need refresh.
            // Optimization: could strictly check (row != newRow).
            if (row != newRow || true) { 
                generateMoveEvents(row, r, isTransposed, isReverse, *this);
            }
        }

        const bool changed = (newBoard != board_);
        if (changed) {
            board_ = newBoard;
            score_ += moveScore;
            if (score_ > highScore_) highScore_ = score_;
        }

        // Restore original orientation if we transposed
        if (isTransposed) transpose();

        if (changed) {
            spawnRandomTile();
        }
        return changed;
    }

    void Board::spawnRandomTile() {
        // Find all empty spots
        std::vector<int> empty;
        for (int i = 0; i < 16; ++i) {
            if (((board_ >> (i * 4)) & 0xF) == 0) empty.push_back(i);
        }

        if (!empty.empty()) {
            // Pick a random empty spot
            const int idx = empty[tfe::utils::RandomGenerator::getInt(0, static_cast<int>(empty.size()) - 1)];
            
            // Pick value (2 or 4) based on probability
            const Tile val = tfe::utils::RandomGenerator::getBool(Config::SPAWN_PROBABILITY_2) ? Config::TILE_EXPONENT_LOW : Config::TILE_EXPONENT_HIGH;

            // Set the bitboard
            board_ |= (static_cast<Bitboard>(val) << (idx * 4));

            // Notify UI
            const int r = idx / 4;
            const int c = idx % 4;
            notifyTileSpawn(r, c, (1 << val));
        }
    }

    bool Board::isGameOver() const {
        // Check if any horizontal moves are possible
        for (int r = 0; r < 4; ++r) {
            const Row row = (board_ >> (r * 16)) & 0xFFFF;
            // If moving Left or Right results in a different row, moves are possible.
            if (LookupTable::moveLeftTable[row] != row) return false;
            if (LookupTable::moveRightTable[row] != row) return false;
        }
        
        // Check if any vertical moves are possible (by transposing and checking as rows)
        const Bitboard t = transpose64(board_);
        for (int r = 0; r < 4; ++r) {
            const Row row = (t >> (r * 16)) & 0xFFFF;
            if (LookupTable::moveLeftTable[row] != row) return false;
            if (LookupTable::moveRightTable[row] != row) return false;
        }

        // No moves possible -> Game Over
        notifyGameOver();
        return true;
    }

    GameState Board::getState() const { return GameState{board_, score_}; }

    void Board::loadState(const GameState& state) {
        board_ = state.board;
        score_ = state.score;
        notifyGameReset(); // Refresh UI
    }

    void Board::addObserver(IGameObserver* observer) { observers_.push_back(observer); }
    void Board::removeObserver(IGameObserver* observer) { std::erase(observers_, observer); }
    
    // --- Notification Helpers ---
    void Board::notifyGameReset() const {
        for (auto* o : observers_) o->onGameReset();
    }
    void Board::notifyGameOver() const {
        for (auto* o : observers_) o->onGameOver();
    }
    void Board::notifyTileSpawn(const int r, const int c, const int value) const {
        for (auto* o : observers_) o->onTileSpawn(r, c, value);
    }
    void Board::notifyTileMove(const int fromR, const int fromC, const int toR, const int toC, const int value) const {
        for (auto* o : observers_) o->onTileMove(fromR, fromC, toR, toC, value);
    }
    void Board::notifyTileMerge(const int r, const int c, const int newValue) const {
        for (auto* o : observers_) o->onTileMerge(r, c, newValue);
    }
}  // namespace tfe::core
