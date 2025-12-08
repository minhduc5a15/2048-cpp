#include "raylib-renderer.h"

#include <cmath>
#include <string>

#include "raylib.h"
#include "theme.h"

// Namespace for GUI components in the Text-based Fantasy Engine (TFE)

namespace tfe::gui {

    /**
     * @brief Easing function for a "back-out" effect.
     *
     * The animation overshoots its target (goes beyond 1.0) and then settles back to the target.
     * This creates a satisfying, slightly bouncy effect.
     * @param x The progress of the animation, from 0.0 to 1.0.
     * @return The eased value.
     */
    float RaylibRenderer::easeOutBack(const float x) {
        constexpr float c1 = 1.70158f;
        constexpr float c3 = c1 + 1.0f;
        return 1.0f + c3 * std::pow(x - 1.0f, 3.0f) + c1 * std::pow(x - 1.0f, 2.0f);
    }

    /**
     * @brief Easing function for a "pop" effect.
     *
     * The animation scales up slightly past its target size (e.g., to 1.2x) and then returns to 1.0x.
     * This is used for the merge animation to make it feel more impactful.
     * @param x The progress of the animation, from 0.0 to 1.0.
     * @return The eased scale factor.
     */
    float RaylibRenderer::easePop(const float x) {
        if (x < 0.5f) return 1.0f + (x * 2.0f) * 0.2f;  // Scale up to 1.2
        return 1.2f - ((x - 0.5f) * 2.0f) * 0.2f;       // Scale back down to 1.0
    }

    /**
     * @brief Constructor for the RaylibRenderer.
     *
     * Initializes the Raylib window, sets the target frames per second (FPS),
     * calculates the size of each cell based on screen dimensions and padding,
     * and initializes the grid for cell animations.
     */
    RaylibRenderer::RaylibRenderer() {
        InitWindow(Theme::SCREEN_WIDTH, Theme::SCREEN_HEIGHT, "2048 - C++ Raylib");
        SetTargetFPS(Theme::TARGET_FPS);

        constexpr int boardSize = 4;
        constexpr float totalPadding = Theme::BOARD_PADDING * 2 + Theme::CELL_PADDING * (boardSize - 1);
        cellSize_ = (Theme::SCREEN_WIDTH - totalPadding) / boardSize;

        // Initialize the animation grid for a 4x4 board.
        cellAnims_.resize(4, std::vector<CellAnim>(4));
    }

    /**
     * @brief Destructor for the RaylibRenderer.
     *
     * Closes the Raylib window when the renderer object is destroyed.
     */
    RaylibRenderer::~RaylibRenderer() { CloseWindow(); }

    /**
     * @brief Checks if the user has requested to close the window (e.g., by clicking the 'X' button).
     * @return True if the window should close, false otherwise.
     */
    bool RaylibRenderer::shouldClose() { return WindowShouldClose(); }

    /**
     * @brief Calculates the pixel X coordinate for a given grid column.
     * @param c The column index (0-based).
     * @return The top-left X position in pixels.
     */
    float RaylibRenderer::getPixelX(const int c) const { return Theme::BOARD_PADDING + c * (cellSize_ + Theme::CELL_PADDING); }

    /**
     * @brief Calculates the pixel Y coordinate for a given grid row.
     * @param r The row index (0-based).
     * @return The top-left Y position in pixels.
     */
    float RaylibRenderer::getPixelY(const int r) const { return Theme::HEADER_HEIGHT + r * (cellSize_ + Theme::CELL_PADDING); }

    /**
     * @brief Triggers a spawn animation (zoom effect) for a tile at a specific grid position.
     * @param r The row of the tile to animate.
     * @param c The column of the tile to animate.
     */
    void RaylibRenderer::triggerSpawn(const int r, const int c) {
        if (r >= 0 && r < 4 && c >= 0 && c < 4) cellAnims_[r][c] = {CellAnim::Spawn, 0.0f};
    }

    /**
     * @brief Triggers a merge animation (pop effect) for a tile at a specific grid position.
     * @param r The row of the tile to animate.
     * @param c The column of the tile to animate.
     * @param value The value of the merged tile (e.g., 8, 16).
     */
    void RaylibRenderer::triggerMerge(const int r, const int c, const int value) {
        if (r >= 0 && r < 4 && c >= 0 && c < 4) {
            cellAnims_[r][c] = {CellAnim::Merge, 0.0f};

            FloatingText ft{};
            ft.value = value;

            ft.x = getPixelX(c) + cellSize_ / 2.0f;
            ft.y = getPixelY(r) + cellSize_ / 2.0f;
            ft.lifeTime = 0.0f;
            ft.maxLifeTime = 0.6f;
            floatingTexts_.push_back(ft);
        }
    }

    /**
     * @brief Adds a tile to the list of moving tiles for the slide animation.
     * @param value The value of the tile (e.g., 2, 4, 8).
     * @param id The unique ID of the tile.
     * @param fromR The starting row.
     * @param fromC The starting column.
     * @param toR The destination row.
     * @param toC The destination column.
     */
    void RaylibRenderer::addMovingTile(const int value, const int id, const int fromR, const int fromC, const int toR, const int toC) {
        MovingTile tile{};
        tile.value = value;
        tile.id = id;
        tile.startX = getPixelX(fromC);
        tile.startY = getPixelY(fromR);
        tile.targetX = getPixelX(toC);
        tile.targetY = getPixelY(toR);
        // Store destination to hide the static tile at the destination, preventing a "ghost" tile from appearing before the animation is complete.
        tile.destR = toR;
        tile.destC = toC;
        tile.progress = 0.0f;
        movingTiles_.push_back(tile);
    }

    /**
     * @brief Updates the progress of all active animations.
     * @param dt The delta time (time since the last frame).
     */
    void RaylibRenderer::updateAnimation(const float dt) {
        // Update slide animations for moving tiles.
        constexpr float slideSpeed = Theme::ANIMATION_SPEED_SLIDE;
        for (auto it = movingTiles_.begin(); it != movingTiles_.end();) {
            it->progress += slideSpeed * dt;
            if (it->progress >= 1.0f) {
                // Animation finished, remove the tile from the moving list.
                it = movingTiles_.erase(it);
            } else {
                ++it;
            }
        }

        // Update scale animations for spawning and merging tiles.
        constexpr float scaleSpeed = Theme::ANIMATION_SPEED_SCALE;
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                auto& anim = cellAnims_[r][c];
                if (anim.type != CellAnim::None) {
                    anim.timer += scaleSpeed * dt;
                    if (anim.timer >= 1.0f) {
                        // Animation finished, reset it.
                        anim.type = CellAnim::None;
                        anim.timer = 0.0f;
                    }
                }
            }
        }
        // Update floating text animations
        for (auto it = floatingTexts_.begin(); it != floatingTexts_.end();) {
            it->lifeTime += dt;
            // Bay lên trên: trừ Y (khoảng 50 pixel mỗi giây)
            it->y -= 50.0f * dt;

            if (it->lifeTime >= it->maxLifeTime) {
                it = floatingTexts_.erase(it);
            } else {
                ++it;
            }
        }
    }

    /**
     * @brief Draws the entire game board, including all tiles and animations.
     * @param board The current state of the game board.
     */
    void RaylibRenderer::draw(const tfe::core::Board& board) const {
        ClearBackground(Theme::BG_COLOR);

        // --- Draw Header ---
        DrawText("2048", Theme::BOARD_PADDING, 20, 60, Theme::TEXT_DARK);

        // Helper lambda to draw a score box
        auto drawScoreBox = [](float x, float y, float width, float height, const char* title, int score) {
            DrawRectangleRounded({x, y, width, height}, 0.2f, 6, Theme::EMPTY_CELL_COLOR);
            const int titleWidth = MeasureText(title, 16);
            DrawText(title, x + (width - titleWidth) / 2, y + 10, 16, Theme::TEXT_DARK);

            const std::string scoreText = std::to_string(score);
            const int scoreWidth = MeasureText(scoreText.c_str(), 30);
            DrawText(scoreText.c_str(), x + (width - scoreWidth) / 2, y + 35, 30, Theme::TEXT_LIGHT);
        };

        // Position and draw the score boxes
        constexpr float boxWidth = 120;
        constexpr float boxHeight = 60;
        constexpr float boxPadding = 10;
        constexpr float bestScoreX = Theme::SCREEN_WIDTH - Theme::BOARD_PADDING - boxWidth;
        constexpr float scoreX = bestScoreX - boxWidth - boxPadding;

        drawScoreBox(scoreX, 20, boxWidth, boxHeight, "SCORE", board.getScore());
        drawScoreBox(bestScoreX, 20, boxWidth, boxHeight, "BEST", board.getHighScore());

        // --- Draw Game Grid ---
        const int size = board.getSize();
        const auto& grid = board.getGrid();

        // Draw static elements: background grid and tiles that are not currently moving.
        for (int r = 0; r < size; ++r) {
            for (int c = 0; c < size; ++c) {
                // Check if a moving tile is headed for this cell. If so, don't draw the static tile here.
                bool isDestination = false;
                for (const auto& mt : movingTiles_) {
                    if (mt.destR == r && mt.destC == c) {
                        isDestination = true;
                        break;
                    }
                }

                const float px = getPixelX(c);
                const float py = getPixelY(r);

                // Always draw the empty cell background.
                DrawRectangleRounded({px, py, cellSize_, cellSize_}, 0.1f, 6, Theme::EMPTY_CELL_COLOR);

                const int val = grid[r][c];
                // Don't draw a tile if it's empty or if a moving tile is about to land here.
                if (val == 0 || isDestination) continue;

                // Calculate scale for spawn/merge animations.
                float scale = 1.0f;
                auto& anim = cellAnims_[r][c];
                if (anim.type == CellAnim::Spawn) {
                    scale = easeOutBack(anim.timer);
                } else if (anim.type == CellAnim::Merge) {
                    scale = easePop(anim.timer);
                }

                // Draw the tile, applying the calculated scale for animations.
                const float currentSize = cellSize_ * scale;
                const float offset = (cellSize_ - currentSize) / 2.0f;
                DrawRectangleRounded({px + offset, py + offset, currentSize, currentSize}, 0.1f, 6, Theme::getTileColor(val));

                // Draw the tile's text value, also scaled.
                std::string text = std::to_string(val);
                const int baseFontSize = (val < 100) ? Theme::FONT_SIZE_LARGE : (val < 1000) ? Theme::FONT_SIZE_MEDIUM : Theme::FONT_SIZE_SMALL;
                int fontSize = baseFontSize * scale;
                if (fontSize < 1) fontSize = 1;

                const int textW = MeasureText(text.c_str(), fontSize);
                DrawText(text.c_str(), px + offset + (currentSize - textW) / 2, py + offset + (currentSize - fontSize) / 2, fontSize,
                         Theme::getTextColor(val));
            }
        }

        // Draw all the tiles that are currently in motion (sliding).
        for (const auto& mt : movingTiles_) {
            // Interpolate position based on animation progress.
            const float currX = mt.startX + (mt.targetX - mt.startX) * mt.progress;
            const float currY = mt.startY + (mt.targetY - mt.startY) * mt.progress;

            const Rectangle rect = {currX, currY, cellSize_, cellSize_};
            DrawRectangleRounded(rect, 0.1f, 6, Theme::getTileColor(mt.value));

            // Draw text for the moving tile.
            std::string text = std::to_string(mt.value);
            const int fontSize = (mt.value < 100) ? 50 : 40;
            const int textW = MeasureText(text.c_str(), fontSize);
            DrawText(text.c_str(), currX + (cellSize_ - textW) / 2, currY + (cellSize_ - fontSize) / 2, fontSize, Theme::getTextColor(mt.value));
        }

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