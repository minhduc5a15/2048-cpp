#pragma once
#include <vector>

#include "game-observer.h"
#include "types.h"

namespace tfe::core {

    /**
     * @class Board
     * @brief Manages the core game logic and state of the 2048 game.
     *
     * The Board class is responsible for:
     * - Storing the game state using an efficient 64-bit Bitboard representation.
     * - Executing moves (Up, Down, Left, Right) and merging tiles.
     * - Managing score and high score.
     * - Spawning new random tiles.
     * - notifying observers (UI, Audio, etc.) about game events.
     */
    class Board {
    public:
        /**
         * @brief Constructs a new Board instance.
         * @param size The dimension of the board (default is 4 for a 4x4 grid).
         */
        explicit Board(int size = 4);

        /**
         * @brief Resets the board to its initial state.
         * Clears the board, resets the score, and spawns initial tiles.
         */
        void reset();

        /**
         * @return The size of the board (always 4 in this implementation).
         */
        int getSize() const { return 4; }

        /**
         * @brief Converts the internal Bitboard state to a user-friendly Grid vector.
         * 
         * Useful for GUI rendering where actual tile values (2, 4, 8...) are needed
         * instead of internal exponents.
         * 
         * @return A 2D vector representing the board's visual state.
         */
        Grid getGrid() const;

        /**
         * @brief Gets the tile exponent at a specific position.
         * @param row Row index (0-3).
         * @param col Column index (0-3).
         * @return The exponent value of the tile.
         */
        Tile getTile(int row, int col) const;
        
        /**
         * @brief Manually sets a tile value at a specific position.
         * @param row Row index.
         * @param col Column index.
         * @param value The exponent value to set.
         */
        void setTile(int row, int col, Tile value); 

        /**
         * @brief Attempts to move tiles in the specified direction.
         * 
         * This method handles shifting and merging of tiles.
         * 
         * @param dir The direction to move.
         * @return True if the move changed the board state, False otherwise.
         */
        bool move(Direction dir);

        /**
         * @brief Spawns a random tile (2 or 4) in an empty cell.
         */
        void spawnRandomTile();

        /**
         * @brief Checks if the game is over (no valid moves remaining).
         * @return True if the game is over, False otherwise.
         */
        bool isGameOver() const;

        /**
         * @return The current game score.
         */
        int getScore() const { return score_; }

        /**
         * @return The highest score achieved in this session.
         */
        int getHighScore() const { return highScore_; }

        /**
         * @return True if the 2048 tile (exponent 11) has been reached.
         */
        bool hasWon() const { return hasReachedWinTile_; }

        // --- Observer Pattern ---

        /**
         * @brief Registers an observer to receive game event notifications.
         * @param observer Pointer to the observer instance.
         */
        void addObserver(IGameObserver* observer);

        /**
         * @brief Removes a registered observer.
         * @param observer Pointer to the observer instance to remove.
         */
        void removeObserver(IGameObserver* observer);

        // --- Save/Load System ---

        /**
         * @brief Captures the current game state (board and score).
         * @return A GameState struct containing the current state.
         */
        GameState getState() const;

        /**
         * @brief Restores the game state from a saved GameState struct.
         * @param state The state to restore.
         */
        void loadState(const GameState& state);

        // --- Notifications (Internal/Subclass use) ---
        // These methods trigger callbacks on registered observers.

        void notifyGameReset() const;
        void notifyGameOver() const;
        void notifyTileSpawn(int r, int c, int value) const;
        void notifyTileMove(int fromR, int fromC, int toR, int toC, int value) const;
        void notifyTileMerge(int r, int c, int newValue) const;

    private:
        /**
         * @brief The core game state packed into a 64-bit integer.
         * 
         * This is the primary source of truth for the board configuration.
         * Rows are packed consecutively in memory.
         */
        Bitboard board_ = 0;

        int score_ = 0;
        int highScore_ = 0;
        bool hasReachedWinTile_ = false;

        std::vector<IGameObserver*> observers_;

        /**
         * @brief Transposes the board (swaps rows and columns).
         * 
         * Used to simplify vertical move calculations by converting them
         * into horizontal move operations.
         */
        void transpose();
    };
}  // namespace tfe::core