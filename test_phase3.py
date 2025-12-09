import py2048
from ai.tuple_network import TupleNetwork
from ai.agent import Agent

print("Init Network...")
net = TupleNetwork()
agent = Agent(net)
board = py2048.Board()

print("AI choosing move...")
move = agent.best_move(board)
print("Move chosen:", move)
