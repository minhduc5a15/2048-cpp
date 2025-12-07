#include "gui-game.h"
#include "raylib.h"

namespace tfe::gui {
    GuiGame::GuiGame() : board_(4), isGameOver_(false)
    {
        // Board tự động init 2 ô random
    }

    void GuiGame::run()
    {
        // renderer_ đã gọi InitWindow trong constructor

        while (!renderer_.shouldClose())
        {
            update();
            draw();
        }
        // renderer_ destructor sẽ gọi CloseWindow
    }

    void GuiGame::handleInput()
    {
        if (isGameOver_)
        {
            // Nếu game over, nhấn Enter để reset
            if (IsKeyPressed(KEY_ENTER))
            {
                board_.reset();
                isGameOver_ = false;
            }
            return;
        }

        bool moved = false;
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
        {
            moved = board_.move(tfe::core::Direction::Up);
        }
        else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
        {
            moved = board_.move(tfe::core::Direction::Down);
        }
        else if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
        {
            moved = board_.move(tfe::core::Direction::Left);
        }
        else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
        {
            moved = board_.move(tfe::core::Direction::Right);
        }

        // Nếu có di chuyển, kiểm tra xem game đã kết thúc chưa
        if (moved)
        {
            if (board_.isGameOver())
            {
                isGameOver_ = true;
            }
        }
    }

    void GuiGame::update()
    {
        handleInput();

        auto currentSpawnPos = board_.getLastSpawnPos();
        // Nếu vị trí spawn thay đổi so với lần cuối ta biết -> Có spawn mới
        if (currentSpawnPos.r != lastSeenSpawnPos_.r ||
            currentSpawnPos.c != lastSeenSpawnPos_.c)
        {
            if (currentSpawnPos.r != -1)
            {
                renderer_.triggerSpawnAnimation(currentSpawnPos.r, currentSpawnPos.c);
            }
            lastSeenSpawnPos_ = currentSpawnPos;
        }

        renderer_.updateAnimation(GetFrameTime());
    }

    void GuiGame::draw()
    {
        BeginDrawing();
        renderer_.draw(board_);

        // Nếu Game Over, vẽ đè layer thông báo lên trên
        if (isGameOver_)
        {
            // Vẽ một lớp phủ bán trong suốt
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                          Fade(RAYWHITE, 0.8f));

            const char* text = "GAME OVER";
            int fontSize = 60;
            int textWidth = MeasureText(text, fontSize);
            DrawText(text, (GetScreenWidth() - textWidth) / 2,
                     GetScreenHeight() / 2 - 50, fontSize, DARKGRAY);

            const char* subText = "Press ENTER to Restart";
            int subFontSize = 20;
            int subTextWidth = MeasureText(subText, subFontSize);
            DrawText(subText, (GetScreenWidth() - subTextWidth) / 2,
                     GetScreenHeight() / 2 + 20, subFontSize, GRAY);
        }

        EndDrawing();
    }
} // namespace tfe::gui
