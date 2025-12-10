#include "board.h"

#include <iostream>

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

    Board::Board(int size) {
        static bool tableInitialized = false;
        if (!tableInitialized) {
            LookupTable::init();

            // Attempt to load weight file.
            if (!LookupTable::loadWeights("build/bin/tuple_weights.bin")) {
                std::cerr << "[Core] Warning: Could not open weights file at build/bin/tuple_weights.bin.";
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

    bool Board::move(const Direction dir) {
        // Normalize to Left/Right. If Up/Down, rotate the board
        if (dir == Direction::Up || dir == Direction::Down) transpose();

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
        }

        const bool changed = (newBoard != board_);
        if (changed) {
            board_ = newBoard;
            score_ += moveScore;
            if (score_ > highScore_) highScore_ = score_;
        }

        // Rotate back if needed
        if (dir == Direction::Up || dir == Direction::Down) transpose();

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
    void Board::notifyTileMove(const int fromR, const int fromC, const int toR, const int toC, const Tile value) const {
        for (auto* o : observers_) o->onTileMove(fromR, fromC, toR, toC, value);
    }
    void Board::notifyTileMerge(const int r, const int c, const Tile newValue) const {
        for (auto* o : observers_) o->onTileMerge(r, c, newValue);
    }
}  // namespace tfe::core