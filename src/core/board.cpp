#include "board.h"

#include "config.h"
#include "lookup_table.h"
#include "score/score-manager.h"
#include "utils/random-generator.h"
#include "bitboard_ops.h"

namespace tfe::core {

    Board::Board(int size) {
        static bool tableInitialized = false;
        if (!tableInitialized) {
            LookupTable::init();
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
                Tile t = getTile(r, c);
                result[r][c] = (t == 0) ? 0 : (1 << t);  // Chuyển mũ thành số thực
            }
        }
        return result;
    }

    Tile Board::getTile(int row, int col) const { return (board_ >> ((row * 16) + (col * 4))) & 0xF; }

    void Board::setTile(int row, int col, Tile value) {
        int shift = (row * 16) + (col * 4);
        board_ &= ~(static_cast<Bitboard>(0xF) << shift);
        board_ |= (static_cast<Bitboard>(value) << shift);
    }

    void Board::transpose() { board_ = BitboardOps::transpose64(board_); }

    bool Board::move(Direction dir) {
        auto [newBoard, moveScore] = BitboardOps::executeMove(board_, dir);

        bool changed = (newBoard != board_);
        if (changed) {
            board_ = newBoard;
            score_ += moveScore;
            if (score_ > highScore_) highScore_ = score_;
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
            int idx = empty[tfe::utils::RandomGenerator::getInt(0, empty.size() - 1)];
            Tile val = tfe::utils::RandomGenerator::getBool(Config::SPAWN_PROBABILITY_2) ? Config::TILE_EXPONENT_LOW : Config::TILE_EXPONENT_HIGH;

            board_ |= (static_cast<Bitboard>(val) << (idx * 4));

            int r = idx / 4;
            int c = idx % 4;
            notifyTileSpawn(r, c, (1 << val));
        }
    }

    bool Board::isGameOver() const {
        if (BitboardOps::executeMove(board_, Direction::Left).first != board_) return false;
        if (BitboardOps::executeMove(board_, Direction::Right).first != board_) return false;
        if (BitboardOps::executeMove(board_, Direction::Up).first != board_) return false;
        if (BitboardOps::executeMove(board_, Direction::Down).first != board_) return false;
        
        notifyGameOver();
        return true;
    }

    GameState Board::getState() const { return GameState{board_, score_}; }

    void Board::loadState(const GameState& state) {
        board_ = state.board;
        score_ = state.score;
        notifyGameReset();
    }

    void Board::addObserver(IGameObserver* o) { observers_.push_back(o); }
    void Board::removeObserver(IGameObserver* o) { std::erase(observers_, o); }
    void Board::notifyGameReset() const {
        for (auto* o : observers_) o->onGameReset();
    }
    void Board::notifyGameOver() const {
        for (auto* o : observers_) o->onGameOver();
    }
    void Board::notifyTileSpawn(int r, int c, int v) const {
        for (auto* o : observers_) o->onTileSpawn(r, c, v);
    }
    void Board::notifyTileMove(int fr, int fc, int tr, int tc, Tile v) const {
        for (auto* o : observers_) o->onTileMove(fr, fc, tr, tc, v);
    }
    void Board::notifyTileMerge(int r, int c, Tile v) const {
        for (auto* o : observers_) o->onTileMerge(r, c, v);
    }
}  // namespace tfe::core