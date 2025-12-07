#include "board.h"

#include <algorithm>
#include <stdexcept>

#include "config.h"
#include "score/score-manager.h"
#include "utils/random-generator.h"

namespace tfe::core {

    Board::Board(const int size) : size_(size), score_(0), hasReachedWinTile_(false) {
        highScore_ = tfe::score::ScoreManager::load_high_score();
        if (size < 2) throw std::invalid_argument("Board size must be at least 2x2");
        reset();
    }

    void Board::reset() {
        grid_.assign(size_, std::vector<Tile>(size_, 0));
        idGrid_.assign(size_, std::vector<int>(size_, 0));
        nextId_ = 1;
        score_ = 0;
        hasReachedWinTile_ = false;
        notifyGameReset();
        spawnRandomTile();
        spawnRandomTile();
    }

    GameState Board::getState() const { return GameState{grid_, score_, idGrid_, nextId_}; }

    void Board::loadState(const GameState& state) {
        // Check size for safety (if the save file has a different board size than the current one)
        if (state.grid.size() != static_cast<size_t>(size_)) {
            // Can throw an error or reset if it doesn't match, here we temporarily reset
            reset();
            return;
        }

        grid_ = state.grid;
        score_ = state.score;
        idGrid_ = state.idGrid;
        nextId_ = state.nextId;

        // Update High Score if the loaded score is greater than the current one
        if (score_ > highScore_) {
            highScore_ = score_;
        }

        // Notify the GUI to redraw the entire new board
        // We use notifyGameReset to make the GUI redraw from scratch
        notifyGameReset();
    }

    void Board::addObserver(tfe::IGameObserver* observer) { observers_.push_back(observer); }

    void Board::removeObserver(tfe::IGameObserver* observer) { std::erase(observers_, observer); }

    int Board::getSize() const { return size_; }
    const Grid& Board::getGrid() const { return grid_; }
    int Board::getTileId(const int row, const int col) const {
        if (row >= 0 && row < size_ && col >= 0 && col < size_) return idGrid_[row][col];
        return 0;
    }

    void Board::setTile(const int row, const int col, const Tile value) {
        if (row >= 0 && row < size_ && col >= 0 && col < size_) {
            grid_[row][col] = value;
            idGrid_[row][col] = (value != 0) ? nextId_++ : 0;
        }
    }

    Tile Board::getTile(const int row, const int col) const {
        if (row >= 0 && row < size_ && col >= 0 && col < size_) return grid_[row][col];
        return -1;
    }

    void Board::compress(std::vector<Tile>& row, std::vector<int>& idRow) const {
        std::vector<Tile> newRow(size_, 0);
        std::vector<int> newIdRow(size_, 0);
        int index = 0;
        for (int i = 0; i < size_; ++i) {
            if (row[i] != 0) {
                newRow[index] = row[i];
                newIdRow[index] = idRow[i];
                index++;
            }
        }
        row = newRow;
        idRow = newIdRow;
    }

    void Board::merge(std::vector<Tile>& row, std::vector<int>& idRow) {
        for (int i = 0; i < size_ - 1; ++i) {
            if (row[i] != 0 && row[i] == row[i + 1]) {
                row[i] *= 2;
                score_ += row[i];
                if (score_ > highScore_) highScore_ = score_;
                if (row[i] == Config::WINNING_TILE) hasReachedWinTile_ = true;

                row[i + 1] = 0;
                idRow[i] = nextId_++;
                idRow[i + 1] = 0;
            }
        }
    }

    bool Board::moveLeft() {
        bool changed = false;
        for (int r = 0; r < size_; ++r) {
            std::vector<Tile> originalRow = grid_[r];
            std::vector<int> originalIdRow = idGrid_[r];

            compress(grid_[r], idGrid_[r]);
            merge(grid_[r], idGrid_[r]);
            compress(grid_[r], idGrid_[r]);

            if (grid_[r] == originalRow) continue;
            changed = true;

            std::vector<bool> merged(size_, false);
            for (int c = 0; c < size_; ++c) {
                if (grid_[r][c] == 0) continue;
                bool found = false;
                for (int oc = 0; oc < size_; ++oc) {
                    if (idGrid_[r][c] == originalIdRow[oc]) {
                        if (c != oc) notifyTileMove(r, oc, r, c, grid_[r][c]);
                        found = true;
                        break;
                    }
                }
                if (!found && !merged[c]) {
                    notifyTileMerge(r, c, grid_[r][c]);
                    merged[c] = true;
                }
            }
        }
        return changed;
    }

    void Board::reverse() {
        for (auto& row : grid_) std::ranges::reverse(row);
        for (auto& row : idGrid_) std::ranges::reverse(row);
    }

    void Board::transpose() {
        for (int i = 0; i < size_; ++i) {
            for (int j = i + 1; j < size_; ++j) {
                std::swap(grid_[i][j], grid_[j][i]);
                std::swap(idGrid_[i][j], idGrid_[j][i]);
            }
        }
    }

    bool Board::move(const Direction dir) {
        clearEffects();
        bool changed = false;
        switch (dir) {
            case Direction::Left:
                changed = moveLeft();
                break;
            case Direction::Right:
                reverse();
                changed = moveLeft();
                reverse();
                break;
            case Direction::Up:
                transpose();
                changed = moveLeft();
                transpose();
                break;
            case Direction::Down:
                transpose();
                reverse();
                changed = moveLeft();
                reverse();
                transpose();
                break;
        }
        if (changed) spawnRandomTile();
        return changed;
    }

    void Board::spawnRandomTile() {
        std::vector<std::pair<int, int>> emptyCells;
        for (int i = 0; i < size_; ++i) {
            for (int j = 0; j < size_; ++j) {
                if (grid_[i][j] == 0) emptyCells.emplace_back(i, j);
            }
        }

        if (!emptyCells.empty()) {
            const int index = utils::RandomGenerator::getInt(0, static_cast<int>(emptyCells.size()) - 1);
            const Tile val = utils::RandomGenerator::getBool(Config::SPAWN_PROBABILITY_2) ? Config::TILE_VALUE_LOW : Config::TILE_VALUE_HIGH;
            const int r = emptyCells[index].first;
            const int c = emptyCells[index].second;

            grid_[r][c] = val;
            idGrid_[r][c] = nextId_++;
            lastSpawnPos_ = {r, c};
            notifyTileSpawn(r, c, val);
        }
    }

    bool Board::isGameOver() const {
        for (int i = 0; i < size_; ++i) {
            for (int j = 0; j < size_; ++j) {
                if (grid_[i][j] == 0) return false;
                if (j < size_ - 1 && grid_[i][j] == grid_[i][j + 1]) return false;
                if (i < size_ - 1 && grid_[i][j] == grid_[i + 1][j]) return false;
            }
        }
        notifyGameOver();
        return true;
    }

    int Board::getScore() const { return score_; }
    int Board::getHighScore() const { return highScore_; }
    bool Board::hasWon() const { return hasReachedWinTile_; }

    void Board::notifyTileSpawn(const int r, const int c, const Tile value) const {
        for (auto* observer : observers_) observer->onTileSpawn(r, c, value);
    }
    void Board::notifyTileMerge(const int r, const int c, const Tile newValue) const {
        for (auto* observer : observers_) observer->onTileMerge(r, c, newValue);
    }
    void Board::notifyTileMove(const int fromR, const int fromC, const int toR, const int toC, const Tile value) const {
        for (auto* observer : observers_) observer->onTileMove(fromR, fromC, toR, toC, value);
    }
    void Board::notifyGameOver() const {
        for (auto* observer : observers_) observer->onGameOver();
    }
    void Board::notifyGameReset() const {
        for (auto* observer : observers_) observer->onGameReset();
    }

}  // namespace tfe::core
