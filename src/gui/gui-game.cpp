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
        // renderer_ đã tự gọi InitWindow trong constructor của nó
        while (!renderer_.shouldClose()) {
            update();
            draw();
        }
        // renderer_ destructor sẽ tự gọi CloseWindow
    }

    void GuiGame::draw() {
        BeginDrawing();
        renderer_.draw(board_);  // Chỉ vẽ bàn cờ

        if (isGameOver_) {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                          Fade(Theme::BG_COLOR, 0.8f));
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

        tfe::core::Direction dir;
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
            // 1. CHỤP ẢNH TRẠNG THÁI CŨ
            std::map<int, TileSnapshot> oldStates;
            int size = board_.getSize();
            for (int r = 0; r < size; ++r) {
                for (int c = 0; c < size; ++c) {
                    int id = board_.getTileId(r, c);
                    if (id != 0)
                        oldStates[id] = {id, board_.getTile(r, c), r, c};
                }
            }

            // 2. DI CHUYỂN
            bool moved = board_.move(dir);

            // 3. XỬ LÝ ANIMATION
            if (moved) {
                auto spawnPos =
                    board_.getLastSpawnPos();  // Vị trí vừa spawn mới (nếu có)

                for (int r = 0; r < size; ++r) {
                    for (int c = 0; c < size; ++c) {
                        int id = board_.getTileId(r, c);
                        if (id == 0) continue;

                        if (oldStates.count(id)) {
                            // Tile cũ -> Check xem có trượt không
                            TileSnapshot old = oldStates[id];
                            if (old.r != r || old.c != c) {
                                renderer_.addMovingTile(old.val, id, old.r,
                                                        old.c, r, c);
                            }
                        } else {
                            // Tile mới (ID mới) -> Có thể là Merge hoặc Spawn
                            // Nếu vị trí này KHÔNG PHẢI là vị trí spawn ngẫu
                            // nhiên -> Chắc chắn là MERGE
                            if (r != spawnPos.r || c != spawnPos.c) {
                                renderer_.triggerMerge(
                                    r, c);  // Kích hoạt hiệu ứng POP
                            }
                        }
                    }
                }
            }
            if (board_.isGameOver()) isGameOver_ = true;
        }

        // 4. SPAWN ANIMATION (Cho ô mới sinh ra ngẫu nhiên)
        auto currentSpawnPos = board_.getLastSpawnPos();
        if (currentSpawnPos.r != lastSeenSpawnPos_.r ||
            currentSpawnPos.c != lastSeenSpawnPos_.c) {
            if (currentSpawnPos.r != -1) {
                renderer_.triggerSpawn(currentSpawnPos.r, currentSpawnPos.c);
            }
            lastSeenSpawnPos_ = currentSpawnPos;
        }
    }
}  // namespace tfe::gui
