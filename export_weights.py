import pickle
import struct
import os
from ai.tuple_network import TupleNetwork


def export_to_binary(pkl_path="ai/weights.pkl", bin_path="build/bin/tuple_weights.bin"):
    if not os.path.exists(pkl_path):
        print(f"Error: Not found {pkl_path}")
        return

    # Load weights từ Python
    net = TupleNetwork(load_path=pkl_path)
    print(f"Loading weights from {pkl_path}...")
    print(f"Total weights: {len(net.weights)}")

    # Tạo thư mục nếu chưa có
    os.makedirs(os.path.dirname(bin_path), exist_ok=True)

    # Ghi ra file binary
    with open(bin_path, "wb") as f:
        # Ghi Header: Số lượng phần tử (uint32) - 65536
        f.write(struct.pack("I", len(net.weights)))

        # Ghi Data: Mảng float
        for w in net.weights:
            f.write(struct.pack("f", w))

    print(f"Successfully exported to {bin_path}")


if __name__ == "__main__":
    export_to_binary()
