#pragma once
#include <vector>

#include "../core/board.h"

namespace tfe::gui {
    struct MovingTile {
        int value;
        int id;
        float startX, startY;
        float targetX, targetY;
        // Lưu tọa độ lưới đích để check bóng ma
        int destR, destC;
        float progress;
    };

    // Trạng thái animation của một ô
    struct CellAnim {
        enum Type { None, Spawn, Merge };

        Type type = None;
        float timer = 0.0f;  // Chạy từ 0.0 -> 1.0
    };

    class RaylibRenderer {
    public:
        RaylibRenderer();
        ~RaylibRenderer();
        static bool shouldClose() ;

        // Hàm draw gọn gàng, không cần tham số hiddenIds từ ngoài
        void draw(const tfe::core::Board& board) const;

        void updateAnimation(float dt);  // Xử lý tất cả animation (Slide, Scale, Pop)

        // Trigger các hiệu ứng
        void triggerSpawn(int r, int c);
        void triggerMerge(int r, int c);

        // Slide Animation
        void addMovingTile(int value, int id, int fromR, int fromC, int toR, int toC);
        bool isAnimating() const { return !movingTiles_.empty(); }

    private:
        float cellSize_;

        // Thay vì chỉ lưu float scale, ta lưu struct CellAnim
        std::vector<std::vector<CellAnim>> cellAnims_;

        std::vector<MovingTile> movingTiles_;

        float getPixelX(int c) const;
        float getPixelY(int r) const;

        // Hàm Easing (Toán học)
        static float easeOutBack(float x);  // Hiệu ứng đàn hồi (vượt quá 1 rồi quay lại)
        static float easePop(float x);      // Hiệu ứng Pop (Lên 1.2 rồi về 1.0)
    };
}  // namespace tfe::gui
