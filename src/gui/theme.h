#pragma once
#include "raylib.h"

namespace tfe::gui {

    struct Theme {
        // Màu nền tổng thể (Background)
        static constexpr Color BG_COLOR = {187, 173, 160, 255};  // #bbada0

        // Màu của ô trống
        static constexpr Color EMPTY_CELL_COLOR = {205, 193, 180, 255};  // #cdc1b4

        // Màu chữ (Dark & Light)
        static constexpr Color TEXT_DARK = {119, 110, 101, 255};   // #776e65
        static constexpr Color TEXT_LIGHT = {249, 246, 242, 255};  // #f9f6f2

        // Kích thước
        static constexpr int SCREEN_WIDTH = 500;
        static constexpr int SCREEN_HEIGHT = 600;  // Dư 100px ở trên cho điểm số
        static constexpr int BOARD_PADDING = 10;
        static constexpr int CELL_PADDING = 10;

        // Header (Khoảng trống phía trên để vẽ điểm số/logo)
        static constexpr int HEADER_HEIGHT = 100;

        // Animation Settings
        static constexpr int TARGET_FPS = 60;
        static constexpr float ANIMATION_SPEED_SLIDE = 8.0f;
        static constexpr float ANIMATION_SPEED_SCALE = 3.0f;

        // Tile Rendering
        static constexpr float TILE_ROUNDNESS = 0.1f;
        static constexpr int TILE_ROUND_SEGMENTS = 6;

        // Font Sizes
        static constexpr int FONT_SIZE_SMALL = 30;   // Cho số >= 1000
        static constexpr int FONT_SIZE_MEDIUM = 40;  // Cho số >= 100
        static constexpr int FONT_SIZE_LARGE = 50;   // Cho số < 100

        // Helper lấy màu theo giá trị Tile
        static Color getTileColor(const int value) {
            switch (value) {
                case 2:
                    return {238, 228, 218, 255};
                case 4:
                    return {237, 224, 200, 255};
                case 8:
                    return {242, 177, 121, 255};
                case 16:
                    return {245, 149, 99, 255};
                case 32:
                    return {246, 124, 95, 255};
                case 64:
                    return {246, 94, 59, 255};
                case 128:
                    return {237, 207, 114, 255};
                case 256:
                    return {237, 204, 97, 255};
                case 512:
                    return {237, 200, 80, 255};
                case 1024:
                    return {237, 197, 63, 255};
                case 2048:
                    return {237, 194, 46, 255};
                default:
                    return {60, 58, 50, 255};  // Dark for >= 4096
            }
        }

        // Helper chọn màu chữ (số nhỏ màu tối, số lớn màu trắng)
        static Color getTextColor(const int value) { return (value <= 4) ? TEXT_DARK : TEXT_LIGHT; }
    };

}  // namespace tfe::gui