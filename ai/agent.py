import py2048
import random

class Agent:
    def __init__(self, network):
        self.net = network

    def best_move(self, board):
        """
        Tìm nước đi tốt nhất cho trạng thái board hiện tại (Expectimax 1 tầng - Greedy).
        """
        best_score = -float('inf')
        best_action = -1
        
        # Thử cả 4 hướng: 0:Up, 1:Down, 2:Left, 3:Right
        for action in range(4):
            # Copy board hiện tại để thử nước đi
            # Chúng ta dùng trick: set_state để clone board
            current_state = board.get_state()
            current_score_val = board.get_score()
            
            # Tạo board tạm để thử
            temp_board = py2048.Board()
            temp_board.set_state(current_state, current_score_val)
            
            # Thử di chuyển (map int -> Enum Direction)
            # Lưu ý: Cần map đúng với bindings.cpp
            direction = [
                py2048.Direction.Up, 
                py2048.Direction.Down, 
                py2048.Direction.Left, 
                py2048.Direction.Right
            ][action]
            
            changed = temp_board.move(direction)
            
            if changed:
                # Đánh giá trạng thái sau khi đi (Afterstate Value)
                state_val = self.net.get_value(temp_board.get_state())
                # Cộng thêm điểm thưởng từ việc gộp số (score mới - score cũ)
                reward = temp_board.get_score() - current_score_val
                
                total_value = reward + state_val
                
                if total_value > best_score:
                    best_score = total_value
                    best_action = direction
        
        return best_action
