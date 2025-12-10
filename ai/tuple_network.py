import pickle
import os


class TupleNetwork:
    def __init__(self, load_path=None):
        # Weight table: 65536 states (16-bit row)
        self.row_weights = [0.0] * 65536
        self.square_weights = [0.0] * 65536

        if load_path and os.path.exists(load_path):
            self.load(load_path)

    def get_value(self, board_state):
        score = 0.0

        # 1. Calculate score for 4 horizontal rows
        for i in range(0, 64, 16):
            score += self.row_weights[(board_state >> i) & 0xFFFF]

        # 2. Calculate score for 4 vertical columns (by transposing the board)
        t_board = self._transpose(board_state)
        for i in range(0, 64, 16):
            score += self.row_weights[(t_board >> i) & 0xFFFF]
        squares = self._get_squares(board_state)
        for sq in squares:
            score += self.square_weights[sq]
        return score

    def update(self, board_state, delta, learning_rate):
        # Update for horizontal rows
        for i in range(0, 64, 16):
            row_idx = (board_state >> i) & 0xFFFF
            self.row_weights[row_idx] += learning_rate * delta
        # Update for vertical columns
        t_board = self._transpose(board_state)
        for i in range(0, 64, 16):
            col_idx = (t_board >> i) & 0xFFFF
            self.row_weights[col_idx] += learning_rate * delta
        # Update for 2x2 squares
        squares = self._get_squares(board_state)
        for sq_idx in squares:
            self.square_weights[sq_idx] += learning_rate * delta

    def _get_squares(self, board):
        """Extracts 9 2x2 squares from the 64-bit bitboard."""
        squares = []
        # Iterate through the first 3 rows and first 3 columns (to form the top-left corner of the square)
        # Starting bit positions for the top-left cell:
        # Row 0: 0, 4, 8
        # Row 1: 16, 20, 24
        # Row 2: 32, 36, 40
        start_indices = [0, 4, 8, 16, 20, 24, 32, 36, 40]

        for i in start_indices:
            # Create a 16-bit index from 4 cells:
            # Cell 1 (Top-Left): i
            # Cell 2 (Top-Right): i + 4
            # Cell 3 (Bottom-Left): i + 16
            # Cell 4 (Bottom-Right): i + 20

            idx = 0
            idx |= (board >> i) & 0xF  # Cell 1 (first 4 bits)
            idx |= ((board >> (i + 4)) & 0xF) << 4  # Cell 2 (next 4 bits)
            idx |= ((board >> (i + 16)) & 0xF) << 8  # Cell 3
            idx |= ((board >> (i + 20)) & 0xF) << 12  # Cell 4
            squares.append(idx)

        return squares

    def _transpose(self, x):
        # 4x4 bitboard transpose algorithm (each cell 4 bits) similar to C++
        a1 = x & 0xF0F00F0FF0F00F0F
        a2 = x & 0x0000F0F00000F0F0
        a3 = x & 0x0F0F00000F0F0000
        a = a1 | (a2 << 12) | (a3 >> 12)

        b1 = a & 0xFF00FF0000FF00FF
        b2 = a & 0x00FF00FF00000000
        b3 = a & 0x00000000FF00FF00
        return b1 | (b2 >> 24) | (b3 << 24)

    def save(self, path):
        # Ensure the directory exists
        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, 'wb') as f:
            pickle.dump((self.row_weights, self.square_weights), f)

    def load(self, path):
        with open(path, 'rb') as f:
            self.row_weights, self.square_weights = pickle.load(f)
