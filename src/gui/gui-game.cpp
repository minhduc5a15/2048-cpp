#include "gui-game.h"

#include "raylib.h"
#include "score/score-manager.h"
#include "theme.h"

namespace tfe::gui {

    GuiGame::GuiGame() : board_(4), renderer_(), isGameOver_(false), currentMoveDirection_(tfe::core::Direction::Up) {
        board_.addObserver(this);
    }

    void GuiGame::run() {
        while (!tfe::gui::RaylibRenderer::shouldClose()) {
            update();
            draw();
        }
    }

    void GuiGame::draw() const {
        BeginDrawing();
        renderer_.draw(board_);

        if (isGameOver_) {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(Theme::BG_COLOR, 0.8f));
            DrawText("GAME OVER", 80, 250, 60, Theme::TEXT_DARK);
            DrawText("Press ENTER to Restart", 120, 320, 20, Theme::TEXT_DARK);
        }
        EndDrawing();
    }

    void GuiGame::update() {
        renderer_.updateAnimation(GetFrameTime());
        if (renderer_.isAnimating()) {
            return;
        }

        if (isGameOver_) {
            if (IsKeyPressed(KEY_ENTER)) {
                board_.reset();
            }
            return;
        }

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
            board_.isGameOver();
        }
    }

    // --- IGameEventListener Implementation ---

    void GuiGame::onTileSpawn(int r, int c, tfe::core::Tile value) {
        (void)value; // This parameter might be used later for different spawn animations
        renderer_.triggerSpawn(r, c);
    }

    void GuiGame::onTileMerge(int r, int c, tfe::core::Tile newValue) {
        (void)newValue; // This parameter might be used later for different merge animations
        // The event coordinates from the Board are relative to a "move left" operation.
        // We must transform them back based on the actual move direction.
        if (currentMoveDirection_ == tfe::core::Direction::Up) {
            renderer_.triggerMerge(c, r);
        } else if (currentMoveDirection_ == tfe::core::Direction::Down) {
            renderer_.triggerMerge(3 - c, r);
        } else if (currentMoveDirection_ == tfe::core::Direction::Right) {
             renderer_.triggerMerge(r, 3 - c);
        } else { // Left
            renderer_.triggerMerge(r, c);
        }
    }

    void GuiGame::onTileMove(int fromR, int fromC, int toR, int toC, tfe::core::Tile value) {
        // The event coordinates from the Board are relative to a "move left" operation.
        // We must transform them back based on the actual move direction.
        if (currentMoveDirection_ == tfe::core::Direction::Up) {
            renderer_.addMovingTile(value, 0, fromC, fromR, toC, toR);
        } else if (currentMoveDirection_ == tfe::core::Direction::Down) {
             renderer_.addMovingTile(value, 0, 3 - fromC, fromR, 3 - toC, toR);
        } else if (currentMoveDirection_ == tfe::core::Direction::Right) {
            renderer_.addMovingTile(value, 0, fromR, 3 - fromC, toR, 3 - toC);
        } else { // Left
            renderer_.addMovingTile(value, 0, fromR, fromC, toR, toC);
        }
    }

    void GuiGame::onGameOver() {
        isGameOver_ = true;
        tfe::score::ScoreManager::save_game(board_.getScore(), board_.hasWon());
    }

    void GuiGame::onGameReset() {
        isGameOver_ = false;
    }

}  // namespace tfe::gui
