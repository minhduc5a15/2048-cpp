#pragma once

#include <vector>
#include "types.h"
#include "game-observer.h"

namespace tfe::core {

    /**
     * @class Board
     * @brief Manages the state and logic of the 2048 game board.
     */
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
        bool isGameOver() const; // Cannot be const as it may fire a notification

        struct Position {
            int r, c;
        };

        Position getLastSpawnPos() const { return lastSpawnPos_; }

        void addObserver(tfe::IGameObserver* observer);
        void removeObserver(tfe::IGameObserver* observer);

        int getScore() const;
        int getHighScore() const;
        bool hasWon() const;

        GameState getState() const;
        void loadState(const GameState& state);

    private:
        int size_;
        Grid grid_;
        int score_;
        int highScore_;
        bool hasReachedWinTile_;

        std::vector<tfe::IGameObserver*> observers_;
        std::vector<std::vector<int>> idGrid_;
        int nextId_ = 1;

        Position lastSpawnPos_ = {-1, -1};
        std::vector<Position> mergedPos_;

        void compress(std::vector<Tile>& row, std::vector<int>& idRow) const;
        void merge(std::vector<Tile>& row, std::vector<int>& idRow);

        void reverse();
        void transpose();
        bool moveLeft();

        // --- Notifier methods ---
        void notifyTileSpawn(int r, int c, Tile value) const;
        void notifyTileMerge(int r, int c, Tile newValue) const;
        void notifyTileMove(int fromR, int fromC, int toR, int toC, Tile value) const;
        void notifyGameOver() const;
        void notifyGameReset() const;

        void clearEffects() {
            mergedPos_.clear();
            lastSpawnPos_ = {-1, -1};
        }
    };
}  // namespace tfe::core
