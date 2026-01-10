#include "gui-game.h"

#include "raylib.h"
#include "score/score-manager.h"
#include "theme.h"

namespace tfe::gui {

    GuiGame::GuiGame() : board_(4), renderer_(), isGameOver_(false), currentMoveDirection_(tfe::core::Direction::Up) {
        // Register this class as an observer to the board
        board_.addObserver(this);
        
        // Try to resume a saved game
        if (const auto state = tfe::core::GameSaver::load(); state.has_value()) {
            board_.loadState(*state);
        }
    }

    void GuiGame::run() {
        // Main Application Loop
        while (!shouldExitApp_) {
            update();
            draw();
        }
    }

    void GuiGame::draw() const {
        BeginDrawing();

        // 1. Draw the game board (tiles, background, score)
        renderer_.draw(board_);

        // 2. Draw Game Over Overlay
        if (isGameOver_) {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(Theme::BG_COLOR, 0.8f));
            DrawText("GAME OVER", 80, 250, 60, Theme::TEXT_DARK);
            DrawText("Press ENTER to Restart", 120, 320, 20, Theme::TEXT_DARK);
        }

        // 3. Draw Exit Dialog (if active and not already game over)
        if (showExitPrompt_ && !isGameOver_) {
            drawExitDialog();
        }
        
        EndDrawing();
    }

    void GuiGame::drawExitDialog() {
        // Dim the background
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.5f));

        // Draw dialog box
        constexpr int boxW = 400;
        constexpr int boxH = 200;
        const int boxX = (GetScreenWidth() - boxW) / 2;
        const int boxY = (GetScreenHeight() - boxH) / 2;

        DrawRectangle(boxX, boxY, boxW, boxH, Fade(Theme::BG_COLOR, 0.95f));
        DrawRectangleLines(boxX, boxY, boxW, boxH, Theme::TEXT_DARK);

        const auto text1 = "Do you want to save?";
        const auto text2 = "[Y] Yes   [N] No";
        const auto text3 = "[ESC] Cancel";

        DrawText(text1, boxX + (boxW - MeasureText(text1, 30)) / 2, boxY + 40, 30, Theme::TEXT_DARK);
        DrawText(text2, boxX + (boxW - MeasureText(text2, 25)) / 2, boxY + 100, 25, Theme::TEXT_DARK);
        DrawText(text3, boxX + (boxW - MeasureText(text3, 20)) / 2, boxY + 150, 20, Theme::TEXT_LIGHT);
    }

    void GuiGame::update() {
        // --- Priority 1: Handle Exit Dialog ---
        if (showExitPrompt_) {
            if (IsKeyPressed(KEY_Y)) {
                tfe::core::GameSaver::save(board_.getState());
                shouldExitApp_ = true;
            } else if (IsKeyPressed(KEY_N)) {
                tfe::core::GameSaver::clearSave();
                shouldExitApp_ = true;
            } else if (IsKeyPressed(KEY_ESCAPE)) {
                showExitPrompt_ = false;
            }
            return; // Block other input while dialog is open
        }

        // --- Priority 2: Check for Window Close Request ---
        if (tfe::gui::RaylibRenderer::shouldClose()) {
            showExitPrompt_ = true;
            return;
        }

        // --- Priority 3: Animation Updates ---
        // Update animations based on delta time
        renderer_.updateAnimation(GetFrameTime());
        
        // Block input if animations are playing (prevents "fast-forwarding" logic without visuals)
        if (renderer_.isAnimating()) {
            return;
        }

        // --- Priority 4: Game Over Handling ---
        if (isGameOver_) {
            if (IsKeyPressed(KEY_ENTER)) {
                board_.reset();
            }
            return;
        }

        // --- Priority 5: Game Input ---
        bool pressed = false;
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            currentMoveDirection_ = tfe::core::Direction::Up;
            pressed = true;
        } else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            currentMoveDirection_ = tfe::core::Direction::Down;
            pressed = true;
        } else if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
            currentMoveDirection_ = tfe::core::Direction::Left;
            pressed = true;
        } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
            currentMoveDirection_ = tfe::core::Direction::Right;
            pressed = true;
        }

        if (pressed) {
            board_.move(currentMoveDirection_);
            // Check game over state immediately after move
            if (board_.isGameOver()) return;
        }
    }

    // --- IGameEventListener Implementation ---
    // These callbacks bridge the Core logic events to the GUI Renderer

    void GuiGame::onTileSpawn(const int r, const int c, const int value) {
        (void)value;  // Unused: currently spawn animation is generic
        renderer_.triggerSpawn(r, c);
    }

    void GuiGame::onTileMerge(const int r, const int c, const int newValue) {
        renderer_.triggerMerge(r, c, newValue);
    }

    void GuiGame::onTileMove(const int fromR, const int fromC, const int toR, const int toC, const int value) {
        renderer_.addMovingTile(value, 0, fromR, fromC, toR, toC);
    }

    void GuiGame::onGameOver() {
        isGameOver_ = true;
        tfe::score::ScoreManager::save_game(board_.getScore(), board_.hasWon());
        // Clean up save file on game over (permadeath style for session resume)
        tfe::core::GameSaver::clearSave();
    }

    void GuiGame::onGameReset() { isGameOver_ = false; }

}  // namespace tfe::gui
