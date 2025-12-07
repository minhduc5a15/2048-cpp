#include "raylib-renderer.h"
#include "raylib.h"
#include "theme.h"
#include <string>

namespace tfe::gui {
    RaylibRenderer::RaylibRenderer()
    {
        InitWindow(Theme::SCREEN_WIDTH, Theme::SCREEN_HEIGHT, "2048 - C++ Raylib");
        SetTargetFPS(60);

        // Tính kích thước ô: (Màn hình - Padding lề - Padding giữa các ô) / Số ô
        int boardSize = 4;
        float totalPadding =
            Theme::BOARD_PADDING * 2 + Theme::CELL_PADDING * (boardSize - 1);
        cellSize_ = (Theme::SCREEN_WIDTH - totalPadding) / boardSize;
        cellScales_.assign(4, std::vector<float>(4, 1.0f));
    }

    RaylibRenderer::~RaylibRenderer() { CloseWindow(); }

    bool RaylibRenderer::shouldClose() const { return WindowShouldClose(); }

    void RaylibRenderer::draw(const tfe::core::Board& board)
    {
        ClearBackground(Theme::BG_COLOR);

        int size = board.getSize();
        const auto& grid = board.getGrid();

        // Vẽ Header
        DrawText("2048", 20, 20, 40, Theme::TEXT_DARK);

        int startY = 100;

        for (int r = 0; r < size; ++r)
        {
            for (int c = 0; c < size; ++c)
            {
                // 1. Tính toán tọa độ gốc của ô trên lưới
                float originalX =
                    Theme::BOARD_PADDING + c * (cellSize_ + Theme::CELL_PADDING);
                float originalY = startY + r * (cellSize_ + Theme::CELL_PADDING);

                int val = grid[r][c];

                // 2. Lấy tỉ lệ scale hiện tại (cho hiệu ứng Animation)
                // Lưu ý: cellScales_ được khởi tạo trong constructor và cập nhật ở
                // updateAnimation
                float scale = cellScales_[r][c];

                // 3. Tính toán kích thước thực tế dựa trên scale
                float currentSize = cellSize_ * scale;

                // Tính offset để ô luôn nằm giữa tâm khi zoom (Center pivot)
                float offset = (cellSize_ - currentSize) / 2.0f;

                float drawX = originalX + offset;
                float drawY = originalY + offset;

                // 4. Vẽ ô nền (Background Tile)
                Rectangle rect = {drawX, drawY, currentSize, currentSize};
                Color color =
                    (val == 0) ? Theme::EMPTY_CELL_COLOR : Theme::getTileColor(val);

                DrawRectangleRounded(rect, 0.1f, 6, color);

                // 5. Vẽ số (Text)
                if (val != 0)
                {
                    std::string text = std::to_string(val);

                    // Font size gốc
                    int baseFontSize = (val < 100) ? 50 : (val < 1000) ? 40 : 30;

                    // Font size sau khi scale (để chữ cũng zoom theo ô)
                    int currentFontSize = baseFontSize * scale;
                    if (currentFontSize < 1)
                        currentFontSize = 1; // Tránh lỗi size <= 0

                    // Đo kích thước text MỚI để căn giữa
                    int textWidth = MeasureText(text.c_str(), currentFontSize);

                    int textX = drawX + (currentSize - textWidth) / 2;
                    int textY = drawY + (currentSize - currentFontSize) / 2;

                    DrawText(text.c_str(), textX, textY, currentFontSize,
                             Theme::getTextColor(val));
                }
            }
        }
    }

    void RaylibRenderer::triggerSpawnAnimation(int r, int c)
    {
        if (r >= 0 && r < 4 && c >= 0 && c < 4)
        {
            cellScales_[r][c] = 0.0f; // Bắt đầu từ 0 để zoom lên
        }
    }

    void RaylibRenderer::updateAnimation(float dt)
    {
        float speed = 10.0f;

        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                if (cellScales_[i][j] < 1.0f)
                {
                    cellScales_[i][j] += speed * dt;
                    if (cellScales_[i][j] > 1.0f)
                        cellScales_[i][j] = 1.0f;
                }
            }
        }
    }
} // namespace tfe::gui
