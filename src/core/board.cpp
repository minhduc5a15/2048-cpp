#include "board.h"

#include <algorithm>
#include <stdexcept>

#include "utils/random-generator.h"

namespace tfe::core {
    Board::Board(int size) : size_(size) {
        if (size < 2)
            throw std::invalid_argument("Board size must be at least 2x2");
        reset();
    }

    void Board::reset() {
        grid_.assign(size_, std::vector<Tile>(size_, 0));
        idGrid_.assign(size_, std::vector<int>(size_, 0));  // Init ID grid
        nextId_ = 1;                                        // Reset ID counter
        spawnRandomTile();
        spawnRandomTile();
    }

    int Board::getSize() const { return size_; }
    const Grid& Board::getGrid() const { return grid_; }

    int Board::getTileId(int row, int col) const {
        if (row >= 0 && row < size_ && col >= 0 && col < size_) {
            return idGrid_[row][col];
        }
        return 0;
    }

    void Board::setTile(int row, int col, Tile value) {
        if (row >= 0 && row < size_ && col >= 0 && col < size_) {
            grid_[row][col] = value;
            // Nếu set tile thủ công (cho test), ta cũng nên set ID
            if (value != 0)
                idGrid_[row][col] = nextId_++;
            else
                idGrid_[row][col] = 0;
        }
    }

    Tile Board::getTile(int row, int col) const {
        if (row >= 0 && row < size_ && col >= 0 && col < size_)
            return grid_[row][col];
        return -1;
    }

    // --- HELPER LOGIC ---

    void Board::compress(std::vector<Tile>& row, std::vector<int>& idRow) {
        std::vector<Tile> newRow(size_, 0);
        std::vector<int> newIdRow(size_, 0);
        int index = 0;
        for (int i = 0; i < size_; ++i) {
            if (row[i] != 0) {
                newRow[index] = row[i];
                newIdRow[index] = idRow[i];  // Di chuyển cả ID theo
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
                row[i + 1] = 0;

                // Khi merge, tạo ra ID mới cho ô kết quả
                idRow[i] = nextId_++;
                idRow[i + 1] = 0;
            }
        }
    }

    bool Board::moveLeft() {
        bool changed = false;
        for (int i = 0; i < size_; ++i) {
            std::vector<Tile> original = grid_[i];
            std::vector<int> originalIds = idGrid_[i];

            compress(grid_[i], idGrid_[i]);
            merge(grid_[i], idGrid_[i]);
            compress(grid_[i], idGrid_[i]);

            // Kiểm tra thay đổi dựa trên cả Value và ID
            if (grid_[i] != original || idGrid_[i] != originalIds) {
                changed = true;
            }
        }
        return changed;
    }

    void Board::reverse() {
        for (int i = 0; i < size_; ++i) {
            std::reverse(grid_[i].begin(), grid_[i].end());
            std::reverse(idGrid_[i].begin(),
                         idGrid_[i].end());  // Reverse cả ID
        }
    }

    void Board::transpose() {
        Grid newGrid(size_, std::vector<Tile>(size_, 0));
        std::vector<std::vector<int>> newIdGrid(size_,
                                                std::vector<int>(size_, 0));
        for (int i = 0; i < size_; ++i) {
            for (int j = 0; j < size_; ++j) {
                newGrid[i][j] = grid_[j][i];
                newIdGrid[i][j] = idGrid_[j][i];  // Transpose cả ID
            }
        }
        grid_ = newGrid;
        idGrid_ = newIdGrid;
    }

    bool Board::move(Direction dir) {
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
                if (grid_[i][j] == 0) emptyCells.push_back({i, j});
            }
        }

        if (!emptyCells.empty()) {
            int index =
                utils::RandomGenerator::getInt(0, emptyCells.size() - 1);
            Tile val = utils::RandomGenerator::getBool(0.9) ? 2 : 4;

            int r = emptyCells[index].first;
            int c = emptyCells[index].second;

            grid_[r][c] = val;
            idGrid_[r][c] = nextId_++;  // Gán ID mới
            lastSpawnPos_ = {r, c};
        }
    }

    bool Board::isGameOver() const {
        // (Giữ nguyên code cũ)
        for (const auto& row : grid_)
            for (Tile t : row)
                if (t == 0) return false;
        for (int i = 0; i < size_; ++i) {
            for (int j = 0; j < size_; ++j) {
                Tile current = grid_[i][j];
                if (j < size_ - 1 && current == grid_[i][j + 1]) return false;
                if (i < size_ - 1 && current == grid_[i + 1][j]) return false;
            }
        }
        return true;
    }
}  // namespace tfe::core
