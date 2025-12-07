#pragma once
#include "types.h"
#include <vector>

namespace tfe::core {

    class Board {
    public:
        explicit Board(int size = 4);
        void reset();
        int getSize() const;
        const Grid& getGrid() const;
        void setTile(int row, int col, Tile value);
        Tile getTile(int row, int col) const;


        // Thực hiện di chuyển theo hướng, trả về true nếu bàn cờ có thay đổi
        bool move(Direction dir);

        // Sinh ra tile mới (2 hoặc 4) tại vị trí trống ngẫu nhiên
        void spawnRandomTile();

        // Kiểm tra game over (không còn nước đi)
        bool isGameOver() const;

    private:
        int size_;
        Grid grid_;

        // Helpers cho thuật toán transform
        void compress(std::vector<Tile>& row);
        void merge(std::vector<Tile>& row);
        bool moveLeft(); // Xử lý logic gốc cho hướng Trái

        void reverse();   // Đảo ngược các hàng
        void transpose(); // Chuyển vị (hàng thành cột)
    };

} // namespace tfe::core