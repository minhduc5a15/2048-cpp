#pragma once
#include <vector>
#include "types.h"

namespace tfe::core {
    class Board {
    public:
        explicit Board(int size = 4);
        void reset();
        int getSize() const;
        const Grid& getGrid() const;
        void setTile(int row, int col, Tile value);
        Tile getTile(int row, int col) const;

        int getTileId(int row, int col) const;

        bool move(Direction dir);
        void spawnRandomTile();
        bool isGameOver() const;

        struct Position {
            int r, c;
        };

        Position getLastSpawnPos() const { return lastSpawnPos_; }

    private:
        int size_;
        Grid grid_;

        std::vector<std::vector<int>> idGrid_;
        int nextId_ = 1;  // Bộ đếm sinh ID

        Position lastSpawnPos_ = {-1, -1};
        std::vector<Position> mergedPos_;

        void compress(std::vector<Tile>& row, std::vector<int>& idRow) const;
        void merge(std::vector<Tile>& row, std::vector<int>& idRow);

        void reverse();
        void transpose();
        bool moveLeft();

        void clearEffects() {
            mergedPos_.clear();
            lastSpawnPos_ = {-1, -1};
        }
    };
}  // namespace tfe::core
