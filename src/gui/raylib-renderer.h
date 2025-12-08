#pragma once
#include <vector>

#include "../core/board.h"

namespace tfe::gui {
    /**
     * @struct MovingTile
     * @brief Represents a tile that is currently in motion (sliding) across the board.
     */
    struct MovingTile {
        int value;
        int id;
        float startX, startY;    // Starting position in pixels.
        float targetX, targetY;  // Target position in pixels.
        int destR, destC;        // Target grid coordinates, used for ghost checking.
        float progress;          // Animation progress from 0.0 to 1.0.
    };

    /**
     * @struct FloatingText
     * @brief Represents a piece of text (e.g., score change) that floats up and fades out.
     */
    struct FloatingText {
        int value;          // The score value (e.g., 8, 16).
        float x, y;         // Current position in pixels.
        float lifeTime;     // Time elapsed since creation.
        float maxLifeTime;  // Maximum duration this text should be visible (in seconds).
    };

    /**
     * @struct CellAnim
     * @brief Represents the animation state of a single grid cell.
     */
    struct CellAnim {
        enum Type { None, Spawn, Merge };

        Type type = None;
        float timer = 0.0f;  // Animation timer, typically from 0.0 to 1.0.
    };

    /**
     * @class RaylibRenderer
     * @brief Manages all rendering and animation for the GUI using the Raylib library.
     *
     * This class is responsible for drawing the board, tiles, and score, as well as
     * handling all visual animations for sliding, spawning, and merging tiles.
     */
    class RaylibRenderer {
    public:
        RaylibRenderer();
        ~RaylibRenderer();

        /**
         * @brief Checks if the game window should be closed.
         * @return True if a close event has been triggered (e.g., closing the window), false otherwise.
         */
        static bool shouldClose();

        /**
         * @brief Draws the entire game state to the screen.
         * @param board The current state of the game board.
         */
        void draw(const tfe::core::Board& board) const;

        /**
         * @brief Updates all ongoing animations based on the elapsed time.
         * @param dt The delta time (time since the last frame).
         */
        void updateAnimation(float dt);

        /**
         * @brief Triggers a spawn animation at a specific cell.
         * @param r The row index of the cell.
         * @param c The column index of the cell.
         */
        void triggerSpawn(int r, int c);

        /**
         * @brief Triggers a merge animation at a specific cell.
         * @param r The row index of the cell.
         * @param c The column index of the cell.
         * @param value The value of the merged tile (e.g., 8, 16).
         */
        void triggerMerge(int r, int c, int value);

        /**
         * @brief Adds a tile to the list of moving tiles to animate its slide.
         * @param value The value of the tile.
         * @param id The unique ID of the tile.
         * @param fromR The starting row.
         * @param fromC The starting column.
         * @param toR The destination row.
         * @param toC The destination column.
         */
        void addMovingTile(int value, int id, int fromR, int fromC, int toR, int toC);

        /**
         * @brief Checks if any animations are currently active.
         * @return True if there are tiles sliding, false otherwise.
         */
        bool isAnimating() const { return !movingTiles_.empty(); }

    private:
        float cellSize_;  // The size of a single cell in pixels.

        // A grid that tracks the animation state of each cell (for spawning/merging).
        std::vector<std::vector<CellAnim>> cellAnims_;

        // A list of all tiles that are currently sliding.
        std::vector<MovingTile> movingTiles_;

        std::vector<FloatingText> floatingTexts_;

        // Helper to convert a column index to a pixel X coordinate.
        float getPixelX(int c) const;
        // Helper to convert a row index to a pixel Y coordinate.
        float getPixelY(int r) const;

        // --- Easing Functions for Animations ---

        // An "overshooting" ease-out function for a bouncy effect.
        static float easeOutBack(float x);
        // A "pop" effect, where the scale goes past 1.0 and then settles.
        static float easePop(float x);
    };
}  // namespace tfe::gui
