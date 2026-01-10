#include "raylib-renderer.h"

#include <cmath>
#include <string>

#include "raylib.h"
#include "theme.h"

// Namespace for GUI components in the Text-based Fantasy Engine (TFE)

namespace tfe::gui {

    /**
     * @brief Easing function: Ease Out Back.
     * 
     * Creates a "bouncy" effect where the value shoots past 1.0 slightly before settling back.
     * Math: f(t) = 1 + c3(t-1)^3 + c1(t-1)^2
     * Used for tile spawn animations to give them a dynamic feel.
     * 
     * @param x Progress (0.0 to 1.0).
     * @return Eased value (can be > 1.0).
     */
    float RaylibRenderer::easeOutBack(const float x) {
        constexpr float c1 = 1.70158f;
        constexpr float c3 = c1 + 1.0f;
        return 1.0f + c3 * std::pow(x - 1.0f, 3.0f) + c1 * std::pow(x - 1.0f, 2.0f);
    }

    /**
     * @brief Easing function: Pop Effect.
     * 
     * Scales up to 1.2x at 50% progress, then scales back down to 1.0x.
     * Used for merge animations to emphasize combination.
     * 
     * @param x Progress (0.0 to 1.0).
     * @return Scale factor (1.0 to 1.2).
     */
    float RaylibRenderer::easePop(const float x) {
        if (x < 0.5f) return 1.0f + (x * 2.0f) * 0.2f;  // Scale up to 1.2
        return 1.2f - ((x - 0.5f) * 2.0f) * 0.2f;       // Scale back down to 1.0
    }

    RaylibRenderer::RaylibRenderer() {
        InitWindow(Theme::SCREEN_WIDTH, Theme::SCREEN_HEIGHT, "2048 - C++ Raylib");
        SetTargetFPS(Theme::TARGET_FPS);

        // Calculate dynamic cell size based on screen width and padding
        constexpr int boardSize = 4;
        constexpr float totalPadding = Theme::BOARD_PADDING * 2 + Theme::CELL_PADDING * (boardSize - 1);
        cellSize_ = (Theme::SCREEN_WIDTH - totalPadding) / boardSize;

        // Initialize animation state grid
        cellAnims_.resize(4, std::vector<CellAnim>(4));
    }

    RaylibRenderer::~RaylibRenderer() { CloseWindow(); }

    bool RaylibRenderer::shouldClose() { return WindowShouldClose(); }

    /**
     * @brief Converts Grid Column Index -> Screen Pixel X.
     */
    float RaylibRenderer::getPixelX(const int c) const { return Theme::BOARD_PADDING + c * (cellSize_ + Theme::CELL_PADDING); }

    /**
     * @brief Converts Grid Row Index -> Screen Pixel Y.
     * Accounts for the header offset.
     */
    float RaylibRenderer::getPixelY(const int r) const { return Theme::HEADER_HEIGHT + r * (cellSize_ + Theme::CELL_PADDING); }

    void RaylibRenderer::triggerSpawn(const int r, const int c) {
        if (r >= 0 && r < 4 && c >= 0 && c < 4) cellAnims_[r][c] = {CellAnim::Spawn, 0.0f};
    }

    void RaylibRenderer::triggerMerge(const int r, const int c, const int value) {
        if (r >= 0 && r < 4 && c >= 0 && c < 4) {
            cellAnims_[r][c] = {CellAnim::Merge, 0.0f};

            // Spawn floating text for score info
            FloatingText ft{};
            ft.value = value;
            ft.x = getPixelX(c) + cellSize_ / 2.0f;
            ft.y = getPixelY(r) + cellSize_ / 2.0f;
            ft.lifeTime = 0.0f;
            ft.maxLifeTime = 0.6f;
            floatingTexts_.push_back(ft);
        }
    }

    void RaylibRenderer::addMovingTile(const int value, const int id, const int fromR, const int fromC, const int toR, const int toC) {
        MovingTile tile{};
        tile.value = value;
        tile.id = id;
        tile.startX = getPixelX(fromC);
        tile.startY = getPixelY(fromR);
        tile.targetX = getPixelX(toC);
        tile.targetY = getPixelY(toR);
        
        // Mark destination coordinates so we can hide the static tile underneath
        // while the moving tile is sliding on top of it.
        tile.destR = toR;
        tile.destC = toC;
        
        tile.progress = 0.0f;
        movingTiles_.push_back(tile);
    }

    void RaylibRenderer::updateAnimation(const float dt) {
        // 1. Update Sliding Tiles
        constexpr float slideSpeed = Theme::ANIMATION_SPEED_SLIDE;
        for (auto it = movingTiles_.begin(); it != movingTiles_.end();) {
            it->progress += slideSpeed * dt;
            if (it->progress >= 1.0f) {
                it = movingTiles_.erase(it);
            } else {
                ++it;
            }
        }

        // 2. Update Cell Animations (Spawn/Merge scale effects)
        constexpr float scaleSpeed = Theme::ANIMATION_SPEED_SCALE;
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
        
        // 3. Update Floating Texts (Score Popups)
        for (auto it = floatingTexts_.begin(); it != floatingTexts_.end();) {
            it->lifeTime += dt;
            it->y -= 50.0f * dt; // Float upwards

            if (it->lifeTime >= it->maxLifeTime) {
                it = floatingTexts_.erase(it);
            } else {
                ++it;
            }
        }
    }

    void RaylibRenderer::draw(const tfe::core::Board& board) const {
        ClearBackground(Theme::BG_COLOR);

        // --- Layer 1: Header & Score ---
        DrawText("2048", Theme::BOARD_PADDING, 20, 60, Theme::TEXT_DARK);

        auto drawScoreBox = [](float x, float y, float width, float height, const char* title, int score) {
            DrawRectangleRounded({x, y, width, height}, 0.2f, 6, Theme::EMPTY_CELL_COLOR);
            const int titleWidth = MeasureText(title, 16);
            DrawText(title, x + (width - titleWidth) / 2, y + 10, 16, Theme::TEXT_DARK);

            const std::string scoreText = std::to_string(score);
            const int scoreWidth = MeasureText(scoreText.c_str(), 30);
            DrawText(scoreText.c_str(), x + (width - scoreWidth) / 2, y + 35, 30, Theme::TEXT_LIGHT);
        };

        constexpr float boxWidth = 120;
        constexpr float boxHeight = 60;
        constexpr float boxPadding = 10;
        constexpr float bestScoreX = Theme::SCREEN_WIDTH - Theme::BOARD_PADDING - boxWidth;
        constexpr float scoreX = bestScoreX - boxWidth - boxPadding;

        drawScoreBox(scoreX, 20, boxWidth, boxHeight, "SCORE", board.getScore());
        drawScoreBox(bestScoreX, 20, boxWidth, boxHeight, "BEST", board.getHighScore());

        // --- Layer 2: Static Grid & Tiles ---
        const int size = board.getSize();
        const auto& grid = board.getGrid();

        for (int r = 0; r < size; ++r) {
            for (int c = 0; c < size; ++c) {
                const float px = getPixelX(c);
                const float py = getPixelY(r);

                // Background Cell
                DrawRectangleRounded({px, py, cellSize_, cellSize_}, 0.1f, 6, Theme::EMPTY_CELL_COLOR);

                // Check if a moving tile is covering this spot. 
                // If so, we skip drawing the static tile to avoid visual artifacts ("ghosting").
                bool isDestination = false;
                for (const auto& mt : movingTiles_) {
                    if (mt.destR == r && mt.destC == c) {
                        isDestination = true;
                        break;
                    }
                }

                const int val = grid[r][c];
                if (val == 0 || isDestination) continue;

                // Calculate Scale for Pop/Spawn
                float scale = 1.0f;
                auto& anim = cellAnims_[r][c];
                if (anim.type == CellAnim::Spawn) {
                    scale = easeOutBack(anim.timer);
                } else if (anim.type == CellAnim::Merge) {
                    scale = easePop(anim.timer);
                }

                // Draw Tile Body
                const float currentSize = cellSize_ * scale;
                const float offset = (cellSize_ - currentSize) / 2.0f;
                DrawRectangleRounded({px + offset, py + offset, currentSize, currentSize}, 0.1f, 6, Theme::getTileColor(val));

                // Draw Tile Text
                std::string text = std::to_string(val);
                const int baseFontSize = (val < 100) ? Theme::FONT_SIZE_LARGE : (val < 1000) ? Theme::FONT_SIZE_MEDIUM : Theme::FONT_SIZE_SMALL;
                int fontSize = baseFontSize * scale;
                if (fontSize < 1) fontSize = 1;

                const int textW = MeasureText(text.c_str(), fontSize);
                DrawText(text.c_str(), px + offset + (currentSize - textW) / 2, py + offset + (currentSize - fontSize) / 2, fontSize,
                         Theme::getTextColor(val));
            }
        }

        // --- Layer 3: Moving Tiles (On top of static grid) ---
        for (const auto& mt : movingTiles_) {
            // Linear Interpolation (Lerp)
            const float currX = mt.startX + (mt.targetX - mt.startX) * mt.progress;
            const float currY = mt.startY + (mt.targetY - mt.startY) * mt.progress;

            const Rectangle rect = {currX, currY, cellSize_, cellSize_};
            DrawRectangleRounded(rect, 0.1f, 6, Theme::getTileColor(mt.value));

            std::string text = std::to_string(mt.value);
            const int fontSize = (mt.value < 100) ? 50 : 40;
            const int textW = MeasureText(text.c_str(), fontSize);
            DrawText(text.c_str(), currX + (cellSize_ - textW) / 2, currY + (cellSize_ - fontSize) / 2, fontSize, Theme::getTextColor(mt.value));
        }

        // --- Layer 4: Floating UI Text ---
        for (const auto& [value, x, y, lifeTime, maxLifeTime] : floatingTexts_) {
            const float alpha = 1.0f - (lifeTime / maxLifeTime);

            Color color = Theme::TEXT_DARK;
            color.a = static_cast<unsigned char>(alpha * 255);

            std::string text = "+" + std::to_string(value);
            constexpr int fontSize = 40;

            const int textW = MeasureText(text.c_str(), fontSize);

            DrawText(text.c_str(), static_cast<int>(x - textW / 2), static_cast<int>(y), fontSize, color);
        }
    }

}  // namespace tfe::gui