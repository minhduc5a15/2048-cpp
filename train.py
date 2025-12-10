import py2048
from ai.tuple_network import TupleNetwork
from ai.agent import Agent
import time
import os

def train(episodes=10000, alpha=0.0025, save_path="ai/weights.pkl", log_interval=100):
    # Load the old network if it exists to continue training
    net = TupleNetwork(load_path=save_path)
    agent = Agent(net)
    board = py2048.Board()

    print(f"Start training {episodes} episodes...")
    print(f"Alpha (Learning Rate): {alpha}")

    start_time = time.time()
    max_score = 0

    for episode in range(1, episodes + 1):
        board.reset()
        current_state = board.get_state()

        while not board.is_game_over():
            # 1. Choose the best move (Greedy)
            action = agent.best_move(board)

            # No more valid moves
            if action == -1:
                break

            # 2. Execute the move
            prev_score = board.get_score()

            # Map action index to Direction enum
            direction = [
                py2048.Direction.Up,
                py2048.Direction.Down,
                py2048.Direction.Left,
                py2048.Direction.Right
            ][action]
            changed = board.move(direction)
            if not changed: # This shouldn't happen if best_move is correct, but just in case
                break

            # 3. Calculate Reward and new State
            reward = board.get_score() - prev_score
            next_state = board.get_state()

            # 4. TD Update: Learn from mistakes/successes
            # V(s) = V(s) + alpha * (reward + V(s') - V(s))
            current_val = net.get_value(current_state)

            if board.is_game_over(): # Game over -> Future value is 0
                target = 0
            else:
                next_val = net.get_value(next_state)
                target = reward + next_val

            delta = target - current_val
            net.update(current_state, delta, alpha)
            current_state = next_state

        # Statistics
        final_score = board.get_score()
        if final_score > max_score:
            max_score = final_score

        if episode % log_interval == 0:
            duration = time.time() - start_time
            ms_per_game = (duration * 1000) / log_interval
            print(f"Episode {episode}/{episodes} | Max: {max_score} | Last: {final_score} | Speed: {ms_per_game:.2f}ms/game")
            net.save(save_path)
            start_time = time.time()

if __name__ == "__main__":
    train(episodes=10000)