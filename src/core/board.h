#pragma once
#include "types.h"

namespace tfe::core {

    class Board {
    public:
        // Constructor cho phép tùy chỉnh kích thước (Mục 4)
        explicit Board(int size = 4);

        // Reset bàn cờ về trạng thái rỗng
        void reset();

        // Lấy kích thước bàn cờ
        int getSize() const;

        // Lấy dữ liệu bàn cờ (read-only để render)
        const Grid& getGrid() const;

        // Đặt giá trị tile tại vị trí cụ thể (dùng cho test hoặc spawn)
        void setTile(int row, int col, Tile value);

        // Lấy giá trị tile
        Tile getTile(int row, int col) const;

    private:
        int size_;
        Grid grid_;
    };

} // namespace tfe::core