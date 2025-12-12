import py2048


class Agent:
    directions = [
        py2048.Direction.Up,
        py2048.Direction.Down,
        py2048.Direction.Left,
        py2048.Direction.Right
    ]

    def __init__(self, network):
        self.net = network
        self.temp_board = py2048.Board()

    def best_move(self, board):
        """
        Find the best move for the current board state (1-ply Expectimax - Greedy).
        """
        best_score = -float('inf')
        best_action_idx = -1

        # Get the current state one time to use it for 4 directions
        current_state = board.get_state()
        current_score_val = board.get_score()

        # Try all 4 directions: 0:Up, 1:Down, 2:Left, 3:Right
        for i, direction in enumerate(self.directions):
            self.temp_board.set_state(current_state, current_score_val)
            changed = self.temp_board.move(direction)

            if changed:
                # Evaluate the state after the move (Afterstate Value)
                state_val = self.net.get_value(self.temp_board.get_state())
                # Add bonus points from merging tiles (new score - old score)
                reward = self.temp_board.get_score() - current_score_val

                total_value = reward + state_val
                if total_value > best_score:
                    best_score = total_value
                    best_action_idx = i

        return best_action_idx
