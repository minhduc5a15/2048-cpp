import py2048
import random

class Agent:
    def __init__(self, network):
        self.net = network

    def best_move(self, board):
        """
        Find the best move for the current board state (1-ply Expectimax - Greedy).
        """
        best_score = -float('inf')
        best_action = -1
        
        # Try all 4 directions: 0:Up, 1:Down, 2:Left, 3:Right
        for action in range(4):
            # Copy the current board to try the move
            # We use the trick: set_state to clone the board
            current_state = board.get_state()
            current_score_val = board.get_score()
            
            # Create a temporary board to try
            temp_board = py2048.Board()
            temp_board.set_state(current_state, current_score_val)
            
            # Try moving (map int -> Enum Direction)
            # Note: Must map correctly with bindings.cpp
            direction = [
                py2048.Direction.Up, 
                py2048.Direction.Down, 
                py2048.Direction.Left, 
                py2048.Direction.Right
            ][action]
            
            changed = temp_board.move(direction)
            
            if changed:
                # Evaluate the state after the move (Afterstate Value)
                state_val = self.net.get_value(temp_board.get_state())
                # Add bonus points from merging tiles (new score - old score)
                reward = temp_board.get_score() - current_score_val
                
                total_value = reward + state_val
                
                if total_value > best_score:
                    best_score = total_value
                    best_action = direction
        
        return best_action
