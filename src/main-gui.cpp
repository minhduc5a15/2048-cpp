#include "gui/raylib-renderer.h"
#include "core/board.h"

int main() {
    tfe::gui::RaylibRenderer renderer;
    tfe::core::Board board(4);

    // Setup dữ liệu giả để test hiển thị
    // board.reset() đã random 2 ô.
    board.setTile(0, 0, 2);
    board.setTile(0, 1, 4);
    board.setTile(0, 2, 8);
    board.setTile(0, 3, 16);
    board.setTile(1, 0, 32);
    board.setTile(1, 1, 64);
    board.setTile(1, 2, 128);
    board.setTile(1, 3, 2048); // Check màu vàng gold

    while (!renderer.shouldClose()) {
        renderer.draw(board);
    }

    return 0;
}