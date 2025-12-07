#include "gui-game.h"

#include <map>

#include "raylib.h"
#include "theme.h"

namespace tfe::gui {

    struct TileSnapshot {
        int id;
        int val;
        int r, c;
    };

    GuiGame::GuiGame() : board_(4), isGameOver_(false) {}

    void GuiGame::run() {
        while (!renderer_.shouldClose()) {
            update();
            draw();
        }
    }

    void GuiGame::draw() {
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
        if (renderer_.isAnimating()) return;

        if (isGameOver_) {
            if (IsKeyPressed(KEY_ENTER)) {
                board_.reset();
                isGameOver_ = false;
            }
            return;
        }

        tfe::core::Direction dir = {};
        bool pressed = false;
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            dir = tfe::core::Direction::Up;
            pressed = true;
        } else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            dir = tfe::core::Direction::Down;
            pressed = true;
        } else if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
            dir = tfe::core::Direction::Left;
            pressed = true;
        } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
            dir = tfe::core::Direction::Right;
            pressed = true;
        }

        if (pressed) {
            std::map<int, TileSnapshot> oldStates;
            const int size = board_.getSize();
            for (int r = 0; r < size; ++r) {
                for (int c = 0; c < size; ++c) {
                    if (int id = board_.getTileId(r, c); id != 0) oldStates[id] = {id, board_.getTile(r, c), r, c};
                }
            }

            const bool moved = board_.move(dir);

            if (moved) {
                auto spawnPos = board_.getLastSpawnPos();

                for (int r = 0; r < size; ++r) {
                    for (int c = 0; c < size; ++c) {
                        int id = board_.getTileId(r, c);
                        if (id == 0) continue;

                        if (oldStates.contains(id)) {
                            // Tile cũ -> Check xem có trượt không
                            TileSnapshot old = oldStates[id];
                            if (old.r != r || old.c != c) {
                                renderer_.addMovingTile(old.val, id, old.r, old.c, r, c);
                            }
                        } else {
                            if (r != spawnPos.r || c != spawnPos.c) {
                                renderer_.triggerMerge(r, c);
                            }
                        }
                    }
                }
            }
            if (board_.isGameOver()) isGameOver_ = true;
        }

        if (const auto currentSpawnPos = board_.getLastSpawnPos();
            currentSpawnPos.r != lastSeenSpawnPos_.r || currentSpawnPos.c != lastSeenSpawnPos_.c) {
            if (currentSpawnPos.r != -1) {
                renderer_.triggerSpawn(currentSpawnPos.r, currentSpawnPos.c);
            }
            lastSeenSpawnPos_ = currentSpawnPos;
        }
    }
}  // namespace tfe::gui
