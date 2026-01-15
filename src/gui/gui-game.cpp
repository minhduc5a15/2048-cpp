#include "gui-game.h"

#include "ai/expectimax_agent.h"
#include "core/game-saver.h"
#include "raylib.h"
#include "score/score-manager.h"
#include "theme.h"

namespace tfe::gui {

    GuiGame::GuiGame() : board_(), renderer_(), isGameOver_(false), currentMoveDirection_(tfe::core::Direction::Up) {
        board_.addObserver(this);
        if (const auto state = tfe::core::GameSaver::load(); state.has_value()) {
            board_.loadState(*state);
        }
        aiAgent_ = std::make_unique<tfe::ai::ExpectimaxAgent>();
    }

    void GuiGame::run() {
        while (!shouldExitApp_) {
            update();
            draw();
        }
    }

    void GuiGame::draw() const {
        BeginDrawing();
        renderer_.draw(board_);

        if (isGameOver_) {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(Theme::BG_COLOR, 0.8f));

            const char* title = "GAME OVER";
            const int titleSize = 60;
            const int titleWidth = MeasureText(title, titleSize);
            DrawText(title, (GetScreenWidth() - titleWidth) / 2, GetScreenHeight() / 2 - 50, titleSize, Theme::TEXT_DARK);

            const char* subtitle = "Press ENTER to Restart";
            const int subtitleSize = 20;
            const int subtitleWidth = MeasureText(subtitle, subtitleSize);
            DrawText(subtitle, (GetScreenWidth() - subtitleWidth) / 2, GetScreenHeight() / 2 + 20, subtitleSize, Theme::TEXT_DARK);
        }

        if (isAiActive_ && !isGameOver_) {
            DrawText("AI ACTIVE (Press P to Stop)", 10, 10, 20, RED);
        }

        if (showExitPrompt_ && !isGameOver_) {
            drawExitDialog();
        }
        EndDrawing();
    }

    void GuiGame::drawExitDialog() {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.5f));

        // Draw dialog
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
            return;
        }

        if (tfe::gui::RaylibRenderer::shouldClose()) {
            showExitPrompt_ = true;
            return;
        }

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

        // Toggle AI
        if (IsKeyPressed(KEY_P)) {
            isAiActive_ = !isAiActive_;
        }

        // AI Move Logic
        if (isAiActive_ && !renderer_.isAnimating()) {
            const auto bestMove = aiAgent_->getBestMove(board_.getState().board);
            if (bestMove.has_value()) {
                currentMoveDirection_ = *bestMove;
                board_.move(currentMoveDirection_);
                if (board_.isGameOver()) return;
            }
        }

        bool pressed = false;
        if (!isAiActive_) {
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
                if (board_.isGameOver()) return;
            }
        }
    }

    // --- IGameEventListener Implementation ---

    void GuiGame::onTileSpawn(const int r, const int c, const tfe::core::Tile value) {
        (void)value;  // This parameter might be used later for different spawn animations
        renderer_.triggerSpawn(r, c);
    }

    void GuiGame::onTileMerge(const int r, const int c, const tfe::core::Tile newValue) {
        // The event coordinates from the Board are relative to a "move left" operation.
        if (currentMoveDirection_ == tfe::core::Direction::Up) {
            renderer_.triggerMerge(c, r, newValue);
        } else if (currentMoveDirection_ == tfe::core::Direction::Down) {
            renderer_.triggerMerge(3 - c, r, newValue);
        } else if (currentMoveDirection_ == tfe::core::Direction::Right) {
            renderer_.triggerMerge(r, 3 - c, newValue);
        } else {  // Left
            renderer_.triggerMerge(r, c, newValue);
        }
    }

    void GuiGame::onTileMove(const int fromR, const int fromC, const int toR, const int toC, const tfe::core::Tile value) {
        // The event coordinates from the Board are relative to a "move left" operation.
        if (currentMoveDirection_ == tfe::core::Direction::Up) {
            renderer_.addMovingTile(value, 0, fromC, fromR, toC, toR);
        } else if (currentMoveDirection_ == tfe::core::Direction::Down) {
            renderer_.addMovingTile(value, 0, 3 - fromC, fromR, 3 - toC, toR);
        } else if (currentMoveDirection_ == tfe::core::Direction::Right) {
            renderer_.addMovingTile(value, 0, fromR, 3 - fromC, toR, 3 - toC);
        } else {  // Left
            renderer_.addMovingTile(value, 0, fromR, fromC, toR, toC);
        }
    }

    void GuiGame::onGameOver() {
        isGameOver_ = true;
        tfe::score::ScoreManager::save_game(board_.getScore(), board_.hasWon());
        tfe::core::GameSaver::clearSave();
    }

    void GuiGame::onGameReset() { isGameOver_ = false; }

}  // namespace tfe::gui
