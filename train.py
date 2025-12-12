import py2048
from ai.tuple_network import TupleNetwork
from ai.agent import Agent
import time
import os
import csv
from datetime import datetime


def get_max_tile(board):
    grid = board.get_grid()
    max_val = 0
    for row in grid:
        for val in row:
            if val > max_val:
                max_val = val
    return max_val


def train(episodes=500000, alpha=0.0025, save_path="ai/weights.pkl", log_interval=100):
    # 1. Setup Logging
    os.makedirs("logs", exist_ok=True)
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    log_file_path = f"logs/train_{timestamp}.csv"

    print(f"Logging training details to: {log_file_path}")

    with open(log_file_path, mode='w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(["Episode", "Score", "MaxTile", "Steps", "TimeMs"])

    # 2. Load Network & Agent
    net = TupleNetwork(load_path=save_path)
    agent = Agent(net)

    board = py2048.Board()

    print(f"Start training {episodes} episodes...")
    print(f"Alpha: {alpha}")

    batch_start_time = time.time()
    max_score_batch = 0

    log_buffer = []

    for episode in range(1, episodes + 1):
        board.reset()
        current_state = board.get_state()
        steps = 0
        episode_start = time.time()

        while not board.is_game_over():
            action = agent.best_move(board)
            if action == -1: break

            prev_score = board.get_score()

            # Map action index sang Direction
            direction = [
                py2048.Direction.Up, py2048.Direction.Down,
                py2048.Direction.Left, py2048.Direction.Right
            ][action]

            changed = board.move(direction)
            if not changed: break

            steps += 1

            # TD Learning Update
            reward = board.get_score() - prev_score
            next_state = board.get_state()

            current_val = net.get_value(current_state)
            if board.is_game_over():
                target = 0
            else:
                next_val = net.get_value(next_state)
                target = reward + next_val

            delta = target - current_val
            net.update(current_state, delta, alpha)
            current_state = next_state

        final_score = board.get_score()
        max_tile = get_max_tile(board)
        duration_ms = (time.time() - episode_start) * 1000

        log_buffer.append([episode, final_score, max_tile, steps, round(duration_ms, 2)])

        if final_score > max_score_batch:
            max_score_batch = final_score

        # --- Logging & Saving ---
        if episode % log_interval == 0:
            # 1. console (Summary)
            total_duration = time.time() - batch_start_time
            avg_speed = (total_duration * 1000) / log_interval

            print(f"Ep {episode}/{episodes} | "
                  f"MaxScore: {max_score_batch} | "
                  f"LastScore: {final_score} | "
                  f"LastTile: {max_tile} | "
                  f"Speed: {avg_speed:.2f}ms/game")

            # 2. save weights
            net.save(save_path)

            # 3. write
            with open(log_file_path, mode='a', newline='') as f:
                writer = csv.writer(f)
                writer.writerows(log_buffer)
            log_buffer = []  # Clear buffer

            # Reset batch stats
            batch_start_time = time.time()
            max_score_batch = 0


if __name__ == "__main__":
    train(episodes=500000)
