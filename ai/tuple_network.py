import pickle
import os


class TupleNetwork:
    def __init__(self, load_path=None):
        # Bảng trọng số: 65536 trạng thái (16-bit row)
        self.weights = [0.0] * 65536

        if load_path and os.path.exists(load_path):
            self.load(load_path)

    def get_value(self, board_state):
        score = 0

        # 1. Tính điểm 4 hàng ngang
        score += self.weights[(board_state >> 0) & 0xFFFF]
        score += self.weights[(board_state >> 16) & 0xFFFF]
        score += self.weights[(board_state >> 32) & 0xFFFF]
        score += self.weights[(board_state >> 48) & 0xFFFF]

        # 2. Tính điểm 4 cột dọc (bằng cách xoay bàn cờ)
        t_board = self._transpose(board_state)
        score += self.weights[(t_board >> 0) & 0xFFFF]
        score += self.weights[(t_board >> 16) & 0xFFFF]
        score += self.weights[(t_board >> 32) & 0xFFFF]
        score += self.weights[(t_board >> 48) & 0xFFFF]

        return score

    def update(self, board_state, delta, learning_rate):
        # Cập nhật cho hàng ngang
        for i in range(0, 64, 16):
            row_idx = (board_state >> i) & 0xFFFF
            self.weights[row_idx] += learning_rate * delta

        # Cập nhật cho cột dọc
        t_board = self._transpose(board_state)
        for i in range(0, 64, 16):
            col_idx = (t_board >> i) & 0xFFFF
            self.weights[col_idx] += learning_rate * delta

    def _transpose(self, x):
        # Thuật toán xoay bitboard 4x4 (mỗi ô 4 bit) tương tự C++
        a1 = x & 0xF0F00F0FF0F00F0F
        a2 = x & 0x0000F0F00000F0F0
        a3 = x & 0x0F0F00000F0F0000
        a = a1 | (a2 << 12) | (a3 >> 12)

        b1 = a & 0xFF00FF0000FF00FF
        b2 = a & 0x00FF00FF00000000
        b3 = a & 0x00000000FF00FF00
        return b1 | (b2 >> 24) | (b3 << 24)

    def save(self, path):
        # Đảm bảo thư mục tồn tại
        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, 'wb') as f:
            pickle.dump(self.weights, f)

    def load(self, path):
        with open(path, 'rb') as f:
            self.weights = pickle.load(f)
