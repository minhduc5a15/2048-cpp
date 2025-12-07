#pragma once
#include "raylib.h"

namespace tfe::gui {

    /**
     * @struct Theme
     * @brief A static struct that holds all constants related to the visual theme of the GUI.
     *
     * This includes colors, dimensions, animation parameters, and helper functions
     * for selecting the correct styles based on game state (e.g., tile values).
     */
    struct Theme {
        // Overall background color
        static constexpr Color BG_COLOR = {187, 173, 160, 255};  // #bbada0

        // Color for an empty cell in the grid
        static constexpr Color EMPTY_CELL_COLOR = {205, 193, 180, 255};  // #cdc1b4

        // Text Colors (Dark & Light)
        static constexpr Color TEXT_DARK = {119, 110, 101, 255};   // #776e65
        static constexpr Color TEXT_LIGHT = {249, 246, 242, 255};  // #f9f6f2

        // Dimensions
        static constexpr int SCREEN_WIDTH = 500;
        static constexpr int SCREEN_HEIGHT = 600;  // Extra 100px at the top for the score
        static constexpr int BOARD_PADDING = 10;
        static constexpr int CELL_PADDING = 10;

        // Header (The space at the top for drawing the score/logo)
        static constexpr int HEADER_HEIGHT = 100;

        // Animation Settings
        static constexpr int TARGET_FPS = 60;
        static constexpr float ANIMATION_SPEED_SLIDE = 8.0f; // Speed for tile sliding animations
        static constexpr float ANIMATION_SPEED_SCALE = 3.0f; // Speed for tile spawn/merge animations

        // Tile Rendering Properties
        static constexpr float TILE_ROUNDNESS = 0.1f;    // Corner roundness for tiles (0.0 to 1.0)
        static constexpr int TILE_ROUND_SEGMENTS = 6;  // Number of segments for rounded corners

        // Font Sizes
        static constexpr int FONT_SIZE_SMALL = 30;   // For numbers >= 1000
        static constexpr int FONT_SIZE_MEDIUM = 40;  // For numbers >= 100
        static constexpr int FONT_SIZE_LARGE = 50;   // For numbers < 100

        /**
         * @brief Helper to get the background color for a tile based on its value.
         * @param value The integer value of the tile.
         * @return The corresponding Raylib Color.
         */
        static Color getTileColor(const int value) {
            switch (value) {
                case 2:
                    return {238, 228, 218, 255}; // #eee4da
                case 4:
                    return {237, 224, 200, 255}; // #ede0c8
                case 8:
                    return {242, 177, 121, 255}; // #f2b179
                case 16:
                    return {245, 149, 99, 255};  // #f59563
                case 32:
                    return {246, 124, 95, 255};  // #f67c5f
                case 64:
                    return {246, 94, 59, 255};   // #f65e3b
                case 128:
                    return {237, 207, 114, 255}; // #edcf72
                case 256:
                    return {237, 204, 97, 255};  // #edcc61
                case 512:
                    return {237, 200, 80, 255};  // #edc850
                case 1024:
                    return {237, 197, 63, 255};  // #edc53f
                case 2048:
                    return {237, 194, 46, 255};  // #edc22e
                default:
                    return {60, 58, 50, 255};    // Dark color for tiles >= 4096
            }
        }

        /**
         * @brief Helper to select the appropriate text color for a tile.
         *
         * Uses a dark font for small tile values and a light font for larger values.
         * @param value The integer value of the tile.
         * @return The corresponding Raylib Color for the text.
         */
        static Color getTextColor(const int value) { return (value <= 4) ? TEXT_DARK : TEXT_LIGHT; }
    };

}  // namespace tfe::gui