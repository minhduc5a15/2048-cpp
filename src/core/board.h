#pragma once
#include <vector>
#include "types.h"

namespace tfe::core {
    /**
     * @class Board
     * @brief Manages the state and logic of the 2048 game board.
     *
     * This class encapsulates the game grid, including all tile manipulation,
     * move handling, and game state validation (e.g., checking for game over).
     */
    class Board {
    public:
        /**
         * @brief Constructs a new game board.
         * @param size The width and height of the square board. Defaults to 4.
         */
        explicit Board(int size = 4);

        /**
         * @brief Resets the board to its initial state (empty grid) and spawns starting tiles.
         */
        void reset();

        /**
         * @brief Gets the size of the board.
         * @return The size (width/height) of the board.
         */
        int getSize() const;

        /**
         * @brief Gets a constant reference to the game grid.
         * @return A const reference to the 2D vector representing the board.
         */
        const Grid& getGrid() const;

        /**
         * @brief Sets the value of a specific tile on the board.
         * @param row The row index of the tile.
         * @param col The column index of the tile.
         * @param value The new value for the tile.
         */
        void setTile(int row, int col, Tile value);

        /**
         * @brief Gets the value of a specific tile on the board.
         * @param row The row index of the tile.
         * @param col The column index of the tile.
         * @return The value of the tile at the specified position.
         */
        Tile getTile(int row, int col) const;

        int getTileId(int row, int col) const;

        /**
         * @brief Executes a move in the specified direction.
         * @param dir The direction to move the tiles (Up, Down, Left, or Right).
         * @return True if the move resulted in a change to the board state, false otherwise.
         */
        bool move(Direction dir);

        /**
         * @brief Spawns a new random tile (either 2 or 4) on an empty spot on the board.
         */
        void spawnRandomTile();

        /**
         * @brief Checks if the game is over.
         * @return True if no more moves are possible, false otherwise.
         */
        bool isGameOver() const;

        // Represents a 2D position on the board.
        struct Position {
            int r, c;
        };

        /**
         * @brief Gets the position of the last tile that was spawned.
         * @return The {row, col} position of the last spawned tile.
         */
        Position getLastSpawnPos() const { return lastSpawnPos_; }

        /**
         * @brief Gets the current score.
         * @return The current score.
         */
        int getScore() const;

        /**
         * @brief Gets the current high score.
         * @return The high score.
         */
        int getHighScore() const;

        /**
         * @brief Checks if the player has achieved the winning tile.
         * @return True if the winning tile is on the board, false otherwise.
         */
        bool hasWon() const;


    private:
        int size_;   // The size of the board (size x size).
        Grid grid_;  // The 2D grid of tile values.
        int score_;
        int highScore_;
        bool hasReachedWinTile_;

        std::vector<std::vector<int>> idGrid_; // Grid to track unique tile IDs for animations.
        int nextId_ = 1;                       // Counter for generating new unique tile IDs.

        Position lastSpawnPos_ = {-1, -1}; // Position of the last spawned tile.
        std::vector<Position> mergedPos_;      // Positions of tiles that were merged in the last move.

        // Slides all tiles in a row to the left, filling empty spaces.
        void compress(std::vector<Tile>& row, std::vector<int>& idRow) const;
        // Merges adjacent identical tiles in a row.
        void merge(std::vector<Tile>& row, std::vector<int>& idRow);

        // Reverses the order of rows and the elements within them.
        void reverse();
        // Transposes the grid (swaps rows and columns).
        void transpose();
        // Core logic for moving all tiles to the left.
        bool moveLeft();

        // Clears any stored positions from the previous move (e.g., merged positions).
        void clearEffects() {
            mergedPos_.clear();
            lastSpawnPos_ = {-1, -1};
        }
    };
}  // namespace tfe::core
