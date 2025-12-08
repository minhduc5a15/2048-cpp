from gymnasium.envs.registration import register
from .env import TwoZeroFourEightEnv

register(
    id="2048-v0",
    entry_point="python.gym2048.env:TwoZeroFourEightEnv",
)