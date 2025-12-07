#include "board.h"
#include <stdexcept>

namespace tfe::core {

    Board::Board(int size) : size_(size) {
        if (size < 2) {
            throw std::invalid_argument("Board size must be at least 2x2");
        }
        reset();
    }

    void Board::reset() {
        // Khởi tạo grid với kích thước size_ x size_, toàn bộ là 0
        grid_.assign(size_, std::vector<Tile>(size_, 0));
    }

    int Board::getSize() const {
        return size_;
    }

    const Grid& Board::getGrid() const {
        return grid_;
    }

    void Board::setTile(int row, int col, Tile value) {
        if (row >= 0 && row < size_ && col >= 0 && col < size_) {
            grid_[row][col] = value;
        }
    }

    Tile Board::getTile(int row, int col) const {
        if (row >= 0 && row < size_ && col >= 0 && col < size_) {
            return grid_[row][col];
        }
        return -1;
    }

} // namespace tfe::core