import sys
import os

# Thêm thư mục hiện tại vào path để python tìm thấy package 'python'
sys.path.append(os.getcwd())

from python.gym2048.env import TwoZeroFourEightEnv
from gymnasium.utils.env_checker import check_env

env = TwoZeroFourEightEnv()
check_env(env)
print("Environment is valid!")