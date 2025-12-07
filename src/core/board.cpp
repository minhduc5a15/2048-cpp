#include "board.h"
#include "utils/random-generator.h"
#include <stdexcept>
#include <algorithm> // std::reverse

namespace tfe::core {

Board::Board(int size) : size_(size) {
    if (size < 2) throw std::invalid_argument("Size must be >= 2");
    reset();
}

void Board::reset() {
    grid_.assign(size_, std::vector<Tile>(size_, 0));
    spawnRandomTile();
    spawnRandomTile();
}

int Board::getSize() const { return size_; }
const Grid& Board::getGrid() const { return grid_; }

void Board::setTile(int row, int col, Tile value) {
    if (row >= 0 && row < size_ && col >= 0 && col < size_) grid_[row][col] = value;
}

Tile Board::getTile(int row, int col) const {
    if (row >= 0 && row < size_ && col >= 0 && col < size_) return grid_[row][col];
    return -1;
}

// --- LOGIC HELPER ---

// Dồn các số về bên trái, loại bỏ số 0 (Ví dụ: [2, 0, 2, 0] -> [2, 2, 0, 0])
void Board::compress(std::vector<Tile>& row) {
    std::vector<Tile> newRow(size_, 0);
    int index = 0;
    for (int i = 0; i < size_; ++i) {
        if (row[i] != 0) {
            newRow[index++] = row[i];
        }
    }
    row = newRow;
}

// Gộp các số giống nhau (Ví dụ: [2, 2, 4, 0] -> [4, 4, 0, 0] sai -> [4, 0, 4, 0] -> compress lại sau)
// Logic chuẩn 2048: Dồn -> Gộp -> Dồn
void Board::merge(std::vector<Tile>& row) {
    for (int i = 0; i < size_ - 1; ++i) {
        if (row[i] != 0 && row[i] == row[i + 1]) {
            row[i] *= 2;
            row[i + 1] = 0;
            // Quan trọng: Tăng i để không gộp chuỗi (ví dụ 2 2 2 2 -> 4 4 chứ không phải 4 2 2 -> 8)
            // Tuy nhiên với vòng lặp for i++, ta chỉ cần đảm bảo logic đúng.
            // Ở đây sau khi gộp, vị trí i+1 thành 0, vòng lặp tiếp theo sẽ xét 0 và i+2, nên an toàn.
        }
    }
}

// Xử lý move hướng Trái
bool Board::moveLeft() {
    bool changed = false;
    for (int i = 0; i < size_; ++i) {
        std::vector<Tile> original = grid_[i];

        compress(grid_[i]);
        merge(grid_[i]);
        compress(grid_[i]); // Dồn lại lần nữa sau khi gộp tạo ra số 0

        if (grid_[i] != original) {
            changed = true;
        }
    }
    return changed;
}

void Board::reverse() {
    for (int i = 0; i < size_; ++i) {
        std::reverse(grid_[i].begin(), grid_[i].end());
    }
}

void Board::transpose() {
    Grid newGrid(size_, std::vector<Tile>(size_, 0));
    for (int i = 0; i < size_; ++i) {
        for (int j = 0; j < size_; ++j) {
            newGrid[i][j] = grid_[j][i];
        }
    }
    grid_ = newGrid;
}

// --- MAIN LOGIC ---

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

    if (changed) {
        spawnRandomTile();
    }
    return changed;
}

void Board::spawnRandomTile() {
    std::vector<std::pair<int, int>> emptyCells;
    for (int i = 0; i < size_; ++i) {
        for (int j = 0; j < size_; ++j) {
            if (grid_[i][j] == 0) {
                emptyCells.push_back({i, j});
            }
        }
    }

    if (!emptyCells.empty()) {
        int index = utils::RandomGenerator::getInt(0, emptyCells.size() - 1);
        Tile val = utils::RandomGenerator::getBool(0.9) ? 2 : 4;

        int r = emptyCells[index].first;
        int c = emptyCells[index].second;
        grid_[r][c] = val;

        lastSpawnPos_ = {r, c};
    }
}

bool Board::isGameOver() const {
    // 1. Còn chỗ trống -> Chưa thua
    for(const auto& row : grid_) {
        for(Tile t : row) if(t == 0) return false;
    }

    // 2. Không còn chỗ trống, kiểm tra xem có ô nào gộp được không (ngang/dọc)
    for (int i = 0; i < size_; ++i) {
        for (int j = 0; j < size_; ++j) {
            Tile current = grid_[i][j];
            // Check phải
            if (j < size_ - 1 && current == grid_[i][j + 1]) return false;
            // Check dưới
            if (i < size_ - 1 && current == grid_[i + 1][j]) return false;
        }
    }

    return true;
}

} // namespace tfe::core