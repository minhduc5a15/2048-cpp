#include "raylib-renderer.h"

#include <cmath>
#include <string>
#include "raylib.h"
#include "theme.h"

namespace tfe::gui {

    // Hiệu ứng nảy: Vượt quá 1.0 rồi thu lại
    float RaylibRenderer::easeOutBack(float x) {
        const float c1 = 1.70158f;
        const float c3 = c1 + 1.0f;
        return 1.0f + c3 * std::pow(x - 1.0f, 3.0f) +
               c1 * std::pow(x - 1.0f, 2.0f);
    }

    // Hiệu ứng Pop: Phồng lên 1.2 rồi về 1.0
    float RaylibRenderer::easePop(float x) {
        if (x < 0.5f) return 1.0f + (x * 2.0f) * 0.2f;
        return 1.2f - ((x - 0.5f) * 2.0f) * 0.2f;
    }

    // --- 2. LOGIC CHÍNH ---

    RaylibRenderer::RaylibRenderer() {
        InitWindow(Theme::SCREEN_WIDTH, Theme::SCREEN_HEIGHT,
                   "2048 - C++ Raylib");
        SetTargetFPS(60);

        int boardSize = 4;
        float totalPadding =
            Theme::BOARD_PADDING * 2 + Theme::CELL_PADDING * (boardSize - 1);
        cellSize_ = (Theme::SCREEN_WIDTH - totalPadding) / boardSize;

        // Khởi tạo lưới animation
        cellAnims_.resize(4, std::vector<CellAnim>(4));
    }

    RaylibRenderer::~RaylibRenderer() { CloseWindow(); }
    bool RaylibRenderer::shouldClose() const { return WindowShouldClose(); }

    float RaylibRenderer::getPixelX(int c) const {
        return Theme::BOARD_PADDING + c * (cellSize_ + Theme::CELL_PADDING);
    }
    float RaylibRenderer::getPixelY(int r) const {
        return 100 + r * (cellSize_ + Theme::CELL_PADDING);
    }

    // Trigger hiệu ứng Spawn (Zoom từ 0 lên 1)
    void RaylibRenderer::triggerSpawn(int r, int c) {
        if (r >= 0 && r < 4 && c >= 0 && c < 4)
            cellAnims_[r][c] = {CellAnim::Spawn, 0.0f};
    }

    // Trigger hiệu ứng Merge (Pop lên 1.2)
    void RaylibRenderer::triggerMerge(int r, int c) {
        if (r >= 0 && r < 4 && c >= 0 && c < 4)
            cellAnims_[r][c] = {CellAnim::Merge, 0.0f};
    }

    void RaylibRenderer::addMovingTile(int value, int id, int fromR, int fromC,
                                       int toR, int toC) {
        MovingTile tile;
        tile.value = value;
        tile.id = id;
        tile.startX = getPixelX(fromC);
        tile.startY = getPixelY(fromR);
        tile.targetX = getPixelX(toC);
        tile.targetY = getPixelY(toR);
        tile.destR = toR;  // Lưu đích đến để ẩn ô tĩnh (chống bóng ma)
        tile.destC = toC;
        tile.progress = 0.0f;
        movingTiles_.push_back(tile);
    }

    void RaylibRenderer::updateAnimation(float dt) {
        // Update Slide
        float slideSpeed = 8.0f;
        for (auto it = movingTiles_.begin(); it != movingTiles_.end();) {
            it->progress += slideSpeed * dt;
            if (it->progress >= 1.0f)
                it = movingTiles_.erase(it);
            else
                ++it;
        }

        // Update Scale (Spawn/Merge)
        float scaleSpeed = 3.0f;  // Tốc độ zoom
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                auto& anim = cellAnims_[r][c];
                if (anim.type != CellAnim::None) {
                    anim.timer += scaleSpeed * dt;
                    if (anim.timer >= 1.0f) {
                        anim.type = CellAnim::None;
                        anim.timer = 0.0f;
                    }
                }
            }
        }
    }

    void RaylibRenderer::draw(const tfe::core::Board& board) {
        ClearBackground(Theme::BG_COLOR);
        DrawText("2048", 20, 20, 40, Theme::TEXT_DARK);

        int size = board.getSize();
        const auto& grid = board.getGrid();

        for (int r = 0; r < size; ++r) {
            for (int c = 0; c < size; ++c) {
                bool isDestination = false;
                for (const auto& mt : movingTiles_) {
                    if (mt.destR == r && mt.destC == c) {
                        isDestination = true;
                        break;
                    }
                }

                float px = getPixelX(c);
                float py = getPixelY(r);

                // Luôn vẽ ô nền (Empty)
                DrawRectangleRounded({px, py, cellSize_, cellSize_}, 0.1f, 6,
                                     Theme::EMPTY_CELL_COLOR);

                int val = grid[r][c];
                if (val == 0 || isDestination) continue;

                // Tính toán Scale đàn hồi
                float scale = 1.0f;
                auto& anim = cellAnims_[r][c];
                if (anim.type == CellAnim::Spawn)
                    scale = easeOutBack(anim.timer);
                else if (anim.type == CellAnim::Merge)
                    scale = easePop(anim.timer);

                // Vẽ Tile
                float currentSize = cellSize_ * scale;
                float offset = (cellSize_ - currentSize) / 2.0f;

                DrawRectangleRounded(
                    {px + offset, py + offset, currentSize, currentSize}, 0.1f,
                    6, Theme::getTileColor(val));

                // Vẽ Text (Scale theo tile)
                std::string text = std::to_string(val);
                int baseFontSize = (val < 100) ? 50 : (val < 1000) ? 40 : 30;
                int fontSize = baseFontSize * scale;
                if (fontSize < 1) fontSize = 1;

                int textW = MeasureText(text.c_str(), fontSize);
                DrawText(text.c_str(), px + offset + (currentSize - textW) / 2,
                         py + offset + (currentSize - fontSize) / 2, fontSize,
                         Theme::getTextColor(val));
            }
        }

        for (const auto& mt : movingTiles_) {
            float currX = mt.startX + (mt.targetX - mt.startX) * mt.progress;
            float currY = mt.startY + (mt.targetY - mt.startY) * mt.progress;

            Rectangle rect = {currX, currY, cellSize_, cellSize_};
            DrawRectangleRounded(rect, 0.1f, 6, Theme::getTileColor(mt.value));

            std::string text = std::to_string(mt.value);
            int fontSize = (mt.value < 100) ? 50 : 40;
            int textW = MeasureText(text.c_str(), fontSize);
            DrawText(text.c_str(), currX + (cellSize_ - textW) / 2,
                     currY + (cellSize_ - fontSize) / 2, fontSize,
                     Theme::getTextColor(mt.value));
        }
    }

}  // namespace tfe::gui