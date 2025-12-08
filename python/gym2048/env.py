import numpy as np
import gymnasium as gym
from gymnasium import spaces

# Import module C++ (nằm cùng thư mục nhờ CMake đã copy sang)
try:
    from . import py2048
except ImportError:
    # Fallback nếu chạy trực tiếp file này
    import py2048

class TwoZeroFourEightEnv(gym.Env):
    metadata = {"render_modes": ["human", "ansi"], "render_fps": 0}

    def __init__(self, size=4, seed=None):
        super().__init__()
        self.board_size = size

        # Khởi tạo game core C++
        self.game = py2048.Board(size)

        # Action Space: 4 hướng (0: Up, 1: Down, 2: Left, 3: Right)
        self.action_space = spaces.Discrete(4)

        # Observation Space: Lưới 4x4.
        # Log scaling: giá trị max khoảng 2^16 -> 16
        self.observation_space = spaces.Box(
            low=0, high=16, shape=(size, size), dtype=np.float32
        )

        self.prev_score = 0

        # Map từ số nguyên sang Enum C++
        self._action_to_direction = {
            0: py2048.Direction.Up,
            1: py2048.Direction.Down,
            2: py2048.Direction.Left,
            3: py2048.Direction.Right,
        }

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)
        if seed is not None:
            py2048.set_seed(seed)

        self.game.reset()
        self.prev_score = 0

        return self._get_obs(), {}

    def step(self, action):
        # 1. Thực hiện hành động
        direction = self._action_to_direction[action]
        changed = self.game.move(direction)

        # 2. Tính toán Reward
        current_score = self.game.get_score()
        reward = float(current_score - self.prev_score)
        self.prev_score = current_score

        # Phạt nhẹ nếu đi vào tường (giúp AI học nhanh hơn)
        if not changed:
            reward = -0.1

        # 3. Kiểm tra kết thúc
        terminated = self.game.is_game_over()
        truncated = False

        return self._get_obs(), reward, terminated, truncated, {}

    def _get_obs(self):
        """Chuyển grid C++ sang numpy array và log scaling"""
        grid = np.array(self.game.get_grid(), dtype=np.float32)
        # Log2 scaling: 2->1, 4->2... 0->0
        return np.where(grid > 0, np.log2(grid), 0.0).astype(np.float32)