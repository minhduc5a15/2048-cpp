import pickle
import struct
import os
from ai.tuple_network import TupleNetwork

def export_to_binary(pkl_path="ai/weights.pkl", bin_path="build/bin/tuple_weights.bin"):
    if not os.path.exists(pkl_path):
        print(f"Error: Not found {pkl_path}")
        return

    net = TupleNetwork(load_path=pkl_path)
    print(f"Loading weights from {pkl_path}...")

    os.makedirs(os.path.dirname(bin_path), exist_ok=True)

    with open(bin_path, "wb") as f:
        # 1. Write row table
        print(f"Writing Row Weights: {len(net.row_weights)}")
        for w in net.row_weights:
            f.write(struct.pack("f", w))

        # 2. Ghi bảng Square
        print(f"Writing Square Weights: {len(net.square_weights)}")
        for w in net.square_weights:
            f.write(struct.pack("f", w))

    print(f"Successfully exported to {bin_path}")

if __name__ == "__main__":
    export_to_binary()