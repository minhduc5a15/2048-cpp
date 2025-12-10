#include "board.h"

#include <algorithm>

#include "config.h"
#include "lookup_table.h"
#include "score/score-manager.h"
#include "utils/random-generator.h"

namespace tfe::core {

    // Rotate 4x4 bitboard (Transpose)
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
        static bool tableInitialized = false;
        if (!tableInitialized) {
            LookupTable::init();

            // Attempt to load weight file.
            // Priority:
            // 1. Same directory (./tuple_weights.bin)
            // 2. Parent directory (../tuple_weights.bin - for when running from build/)
            if (!LookupTable::loadWeights("tuple_weights.bin")) {
                LookupTable::loadWeights("../tuple_weights.bin");
            }

            tableInitialized = true;
        }
        highScore_ = tfe::score::ScoreManager::load_high_score();
        reset();
    }

    void Board::reset() {
        board_ = 0;
        score_ = 0;
        hasReachedWinTile_ = false;
        notifyGameReset();
        spawnRandomTile();
        spawnRandomTile();
    }

    Grid Board::getGrid() const {
        Grid result(4, std::vector<int>(4));
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                const Tile t = getTile(r, c);
                result[r][c] = (t == 0) ? 0 : (1 << t);  // Convert exponent to real number
            }
        }
        return result;
    }

    Tile Board::getTile(const int row, const int col) const { return (board_ >> ((row * 16) + (col * 4))) & 0xF; }

    void Board::setTile(const int row, const int col, const Tile value) {
        const int shift = (row * 16) + (col * 4);
        board_ &= ~(static_cast<Bitboard>(0xF) << shift);
        board_ |= (static_cast<Bitboard>(value) << shift);
    }

    void Board::transpose() { board_ = transpose64(board_); }

    // Helper to calculate move events for animations
    static void generateMoveEvents(const Row row, const int rIdx, const bool isTransposed, const bool isReverse, const Board& board) {
        int cells[4];
        // Unpack row
        for (int i = 0; i < 4; ++i) cells[i] = (row >> (i * 4)) & 0xF;

        // If reverse (Right/Down), we simulate moving RIGHT by processing cells in reverse order or treating it as left move on reversed array
        // Let's stick to "Standard Move Left" logic on the cells array, but if isReverse, we reverse the input first.

        if (isReverse) {
            std::ranges::reverse(cells);
        }

        // Standard Move Left Simulation
        int targetPos = 0;
        bool merged[4] = {false, false, false, false};  // Track merges at target positions
        // Result array to track values at target positions for merge checks
        int resultValues[4] = {0, 0, 0, 0};

        for (int sourcePos = 0; sourcePos < 4; ++sourcePos) {
            const int val = cells[sourcePos];
            if (val == 0) continue;

            // Check merge with previous tile
            if (targetPos > 0 && resultValues[targetPos - 1] == val && !merged[targetPos - 1]) {
                // MERGE
                merged[targetPos - 1] = true;
                resultValues[targetPos - 1]++;                        // Increment exponent
                const int newVal = 1 << resultValues[targetPos - 1];  // Real value

                // Calculate Coordinates
                // We need to map sourcePos and (targetPos-1) back to real board coordinates.

                const int mappedSrc = isReverse ? (3 - sourcePos) : sourcePos;
                const int mappedDst = isReverse ? (3 - (targetPos - 1)) : (targetPos - 1);

                int srcR, srcC, dstR, dstC;
                if (isTransposed) {
                    srcR = mappedSrc;
                    srcC = rIdx;
                    dstR = mappedDst;
                    dstC = rIdx;
                } else {
                    srcR = rIdx;
                    srcC = mappedSrc;
                    dstR = rIdx;
                    dstC = mappedDst;
                }

                // Fire move event for the merging tile
                const int movingVal = 1 << val;
                board.notifyTileMove(srcR, srcC, dstR, dstC, movingVal);

                // Fire merge event at destination
                board.notifyTileMerge(dstR, dstC, newVal);

            } else {
                // MOVE
                resultValues[targetPos] = val;

                const int mappedSrc = isReverse ? (3 - sourcePos) : sourcePos;
                const int mappedDst = isReverse ? (3 - targetPos) : targetPos;

                int srcR, srcC, dstR, dstC;
                if (isTransposed) {
                    srcR = mappedSrc;
                    srcC = rIdx;
                    dstR = mappedDst;
                    dstC = rIdx;
                } else {
                    srcR = rIdx;
                    srcC = mappedSrc;
                    dstR = rIdx;
                    dstC = mappedDst;
                }

                if (mappedSrc != mappedDst) {
                    const int movingVal = 1 << val;
                    board.notifyTileMove(srcR, srcC, dstR, dstC, movingVal);
                }

                targetPos++;
            }
        }
    }

    bool Board::move(const Direction dir) {
        const bool isTransposed = (dir == Direction::Up || dir == Direction::Down);
        const bool isReverse = (dir == Direction::Right || dir == Direction::Down);

        // Normalize to Left/Right. If Up/Down, rotate the board
        if (isTransposed) transpose();

        Bitboard newBoard = 0;
        int moveScore = 0;

        for (int r = 0; r < 4; ++r) {
            const Row row = (board_ >> (r * 16)) & Config::ROW_MASK;
            Row newRow;

            // Table lookup
            if (dir == Direction::Left || dir == Direction::Up)
                newRow = LookupTable::moveLeftTable[row];
            else
                newRow = LookupTable::moveRightTable[row];

            moveScore += LookupTable::scoreTable[row];
            newBoard |= (static_cast<Bitboard>(newRow) << (r * 16));

            // Generate Animation Events
            // We calculate events based on the *difference* between 'row' and 'newRow' logic
            if (row != newRow || true) {  // Always check to be safe, or optimize
                // Note: generateMoveEvents re-simulates the move to find *which* tile moved *where*.
                // This is necessary because the bitboard/lookup table doesn't store "history".
                generateMoveEvents(row, r, isTransposed, isReverse, *this);
            }
        }

        const bool changed = (newBoard != board_);
        if (changed) {
            board_ = newBoard;
            score_ += moveScore;
            if (score_ > highScore_) highScore_ = score_;
        }

        // Rotate back if needed
        if (isTransposed) transpose();

        if (changed) {
            spawnRandomTile();
        }
        return changed;
    }

    void Board::spawnRandomTile() {
        std::vector<int> empty;
        for (int i = 0; i < 16; ++i) {
            if (((board_ >> (i * 4)) & 0xF) == 0) empty.push_back(i);
        }
        if (!empty.empty()) {
            const int idx = empty[tfe::utils::RandomGenerator::getInt(0, static_cast<int>(empty.size()) - 1)];
            const Tile val = tfe::utils::RandomGenerator::getBool(Config::SPAWN_PROBABILITY_2) ? Config::TILE_EXPONENT_LOW : Config::TILE_EXPONENT_HIGH;

            board_ |= (static_cast<Bitboard>(val) << (idx * 4));

            const int r = idx / 4;
            const int c = idx % 4;
            notifyTileSpawn(r, c, (1 << val));
        }
    }

    bool Board::isGameOver() const {
        // Check horizontal rows
        for (int r = 0; r < 4; ++r) {
            const Row row = (board_ >> (r * 16)) & 0xFFFF;
            if (LookupTable::moveLeftTable[row] != row) return false;
            if (LookupTable::moveRightTable[row] != row) return false;
        }
        // Check vertical columns (rotate then check like horizontal)
        const Bitboard t = transpose64(board_);
        for (int r = 0; r < 4; ++r) {
            const Row row = (t >> (r * 16)) & 0xFFFF;
            if (LookupTable::moveLeftTable[row] != row) return false;
            if (LookupTable::moveRightTable[row] != row) return false;
        }
        notifyGameOver();
        return true;
    }

    GameState Board::getState() const { return GameState{board_, score_}; }

    void Board::loadState(const GameState& state) {
        board_ = state.board;
        score_ = state.score;
        notifyGameReset();
    }

    void Board::addObserver(IGameObserver* observer) { observers_.push_back(observer); }
    void Board::removeObserver(IGameObserver* observer) { std::erase(observers_, observer); }
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
