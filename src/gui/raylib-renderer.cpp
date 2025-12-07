#include "raylib-renderer.h"
#include "raylib.h"
#include "theme.h"
#include <string>

namespace tfe::gui {

RaylibRenderer::RaylibRenderer() {
    InitWindow(Theme::SCREEN_WIDTH, Theme::SCREEN_HEIGHT, "2048 - C++ Raylib");
    SetTargetFPS(60);

    // Tính kích thước ô: (Màn hình - Padding lề - Padding giữa các ô) / Số ô
    int boardSize = 4;
    float totalPadding = Theme::BOARD_PADDING * 2 + Theme::CELL_PADDING * (boardSize - 1);
    cellSize_ = (Theme::SCREEN_WIDTH - totalPadding) / boardSize;
}

RaylibRenderer::~RaylibRenderer() {
    CloseWindow();
}

bool RaylibRenderer::shouldClose() const {
    return WindowShouldClose();
}

void RaylibRenderer::draw(const tfe::core::Board& board) {
    ClearBackground(Theme::BG_COLOR);

    int size = board.getSize();
    const auto& grid = board.getGrid();

    // Vẽ Header (Điểm số / Title) - Tạm thời vẽ text đơn giản
    DrawText("2048", 20, 20, 40, Theme::TEXT_DARK);

    // Offset Y để vẽ bàn cờ thấp xuống (chừa chỗ cho Header)
    int startY = 100;

    for (int r = 0; r < size; ++r) {
        for (int c = 0; c < size; ++c) {
            // Tính tọa độ x, y
            float x = Theme::BOARD_PADDING + c * (cellSize_ + Theme::CELL_PADDING);
            float y = startY + r * (cellSize_ + Theme::CELL_PADDING);

            int val = grid[r][c];

            // 1. Vẽ ô nền
            Rectangle rect = {x, y, cellSize_, cellSize_};
            Color color = (val == 0) ? Theme::EMPTY_CELL_COLOR : Theme::getTileColor(val);

            // Roundness 0.1f, segments 6 (độ mượt góc)
            DrawRectangleRounded(rect, 0.1f, 6, color);

            // 2. Vẽ số (nếu khác 0)
            if (val != 0) {
                std::string text = std::to_string(val);
                // Tính font size động: Số càng to font càng bé để vừa ô
                int fontSize = (val < 100) ? 50 : (val < 1000) ? 40 : 30;

                // Đo kích thước text để căn giữa
                int textWidth = MeasureText(text.c_str(), fontSize);
                int textX = x + (cellSize_ - textWidth) / 2;
                int textY = y + (cellSize_ - fontSize) / 2;

                DrawText(text.c_str(), textX, textY, fontSize, Theme::getTextColor(val));
            }
        }
    }
}

} // namespace tfe::gui