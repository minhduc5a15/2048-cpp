#pragma once
#include <vector>

#include "../core/board.h"

namespace tfe::gui {
    /**
     * @struct MovingTile
     * @brief Represents a tile that is currently in motion (sliding) across the board.
     * 
     * Used to render smooth transitions between board states. The core logic moves
     * tiles instantaneously, but this struct allows the GUI to interpolate the position
     * over time.
     */
    struct MovingTile {
        int value;               // The numeric value of the tile (e.g., 2, 4, 8).
        int id;                  // Unique identifier (unused in simple implementation but good for tracking).
        float startX, startY;    // Starting pixel coordinates.
        float targetX, targetY;  // Destination pixel coordinates.
        int destR, destC;        // Destination grid coordinates (to match with static board).
        float progress;          // Animation progress (0.0 to 1.0).
    };

    /**
     * @struct FloatingText
     * @brief Represents a visual score popup (e.g., "+4") that floats upwards and fades out.
     */
    struct FloatingText {
        int value;          // The score value to display.
        float x, y;         // Current pixel coordinates.
        float lifeTime;     // How long the text has been alive (seconds).
        float maxLifeTime;  // Total duration before it disappears.
    };

    /**
     * @struct CellAnim
     * @brief Tracks the animation state for a specific grid cell (Spawn or Merge effects).
     */
    struct CellAnim {
        enum Type { None, Spawn, Merge };

        Type type = None;    // The type of animation currently playing.
        float timer = 0.0f;  // Animation progress timer.
    };

    /**
     * @class RaylibRenderer
     * @brief Handles all graphical rendering using the Raylib library.
     *
     * Responsibilities:
     * - Initializing and closing the Raylib window context.
     * - Drawing the static board grid and background.
     * - Managing and drawing active animations (sliding tiles, pop-ups).
     * - Handling window events (close request).
     */
    class RaylibRenderer {
    public:
        /**
         * @brief Initializes the Raylib window and loads font resources.
         */
        RaylibRenderer();

        /**
         * @brief Closes the Raylib window and unloads resources.
         */
        ~RaylibRenderer();

        /**
         * @brief Checks if the user has requested to close the window (e.g., clicked X).
         * @return True if the window should close.
         */
        static bool shouldClose();

        /**
         * @brief Main draw function. Renders the entire game scene.
         * @param board The current state of the game board.
         */
        void draw(const tfe::core::Board& board) const;

        /**
         * @brief Updates the state of all active animations.
         * @param dt Delta time (time elapsed since last frame) in seconds.
         */
        void updateAnimation(float dt);

        /**
         * @brief Triggers a "Spawn" animation (scaling up from 0) at a specific cell.
         */
        void triggerSpawn(int r, int c);

        /**
         * @brief Triggers a "Merge" animation (pop effect) and adds floating score text.
         */
        void triggerMerge(int r, int c, int value);

        /**
         * @brief Registers a new sliding tile animation.
         * 
         * @param value The tile value.
         * @param id Unique ID.
         * @param fromR Source row.
         * @param fromC Source column.
         * @param toR Destination row.
         * @param toC Destination column.
         */
        void addMovingTile(int value, int id, int fromR, int fromC, int toR, int toC);

        /**
         * @brief Checks if any animations (specifically sliding tiles) are still playing.
         * Used to block user input until animations complete.
         */
        bool isAnimating() const { return !movingTiles_.empty(); }

    private:
        float cellSize_;  // Size of one grid cell in pixels (calculated based on window size).

        // Grid tracking animation states for stationary cells (Spawn/Merge)
        std::vector<std::vector<CellAnim>> cellAnims_;

        // List of currently sliding tiles
        std::vector<MovingTile> movingTiles_;

        // List of active floating score texts
        std::vector<FloatingText> floatingTexts_;

        // --- Coordinate Helpers ---
        float getPixelX(int c) const;
        float getPixelY(int r) const;

        // --- Easing Functions ---
        // Ease-out-back: Goes slightly past target and comes back (bouncy).
        static float easeOutBack(float x);
        
        // Pop effect: Scales up beyond 1.0 then settles back to 1.0.
        static float easePop(float x);
    };
}  // namespace tfe::gui
