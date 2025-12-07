#include "raylib-renderer.h"
#include "raylib.h"

namespace tfe::gui {

    void RaylibRenderer::init() {
        InitWindow(800, 600, "2048 - Raylib C++");
        SetTargetFPS(60);

        while (!WindowShouldClose()) {
            BeginDrawing();
            ClearBackground(RAYWHITE);

            DrawText("Raylib is working!", 190, 200, 40, LIGHTGRAY);
            DrawText("Press ESC to exit", 240, 260, 20, DARKGRAY);

            EndDrawing();
        }

        CloseWindow();
    }

} // namespace tfe::gui