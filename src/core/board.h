#pragma once
#include <vector>

#include "game-observer.h"
#include "types.h"

namespace tfe::core {

    class Board {
    public:
        explicit Board(int size = 4);
        void reset();

        int getSize() const { return 4; }

        // Convert Bitboard to Grid vector for GUI rendering
        Grid getGrid() const;

        // Get exponent value at (row, col)
        Tile getTile(int row, int col) const;
        
        // Set exponent value at (row, col)
        void setTile(int row, int col, Tile value); 

        bool move(Direction dir);
        void spawnRandomTile();
        bool isGameOver() const;

        int getScore() const { return score_; }
        int getHighScore() const { return highScore_; }
        bool hasWon() const { return hasReachedWinTile_; }

        // Observer Pattern
        void addObserver(IGameObserver* observer);
        void removeObserver(IGameObserver* observer);

        // Save/Load
        GameState getState() const;
        void loadState(const GameState& state);

    private:
        Bitboard board_ = 0;  // The only variable containing board data!
        int score_ = 0;
        int highScore_ = 0;
        bool hasReachedWinTile_ = false;

        std::vector<IGameObserver*> observers_;

        // Helper private
        void transpose();

        // Notifications
        void notifyGameReset() const;
        void notifyGameOver() const;
        void notifyTileSpawn(int r, int c, int value) const;
        void notifyTileMove(int fromR, int fromC, int toR, int toC, Tile value) const;
        void notifyTileMerge(int r, int c, Tile newValue) const;
    };
}  // namespace tfe::core