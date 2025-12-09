import pickle
import os

class TupleNetwork:
    def __init__(self, load_path=None):
        # Bảng tra cứu (Lookup Table) cho một hàng 4 ô.
        # Một hàng 4 ô, mỗi ô tối đa 15 (2^15), tổng không gian là 16^4 = 65536 trạng thái.
        # Tuy nhiên, để đơn giản và tổng quát, ta dùng dictionary hoặc list.
        # List nhanh hơn: index là giá trị row (uint16), value là điểm.
        self.weights = [0.0] * 65536 
        
        if load_path and os.path.exists(load_path):
            self.load(load_path)

    def get_value(self, board_state):
        """
        Tính tổng giá trị của bàn cờ dựa trên bảng trọng số hiện tại.
        board_state: uint64 (Bitboard từ C++)
        """
        score = 0
        
        # 2048 có tính đối xứng cao. Chúng ta sẽ lấy 16-bit của từng hàng và cột
        # để tra bảng, sau đó cộng dồn lại.
        
        # Xử lý 4 hàng (Standard orientation)
        score += self.weights[(board_state >> 0) & 0xFFFF]
        score += self.weights[(board_state >> 16) & 0xFFFF]
        score += self.weights[(board_state >> 32) & 0xFFFF]
        score += self.weights[(board_state >> 48) & 0xFFFF]
        
        # Để AI mạnh hơn, ta cần nhìn cả Cột. 
        # Việc xoay bitboard trong Python hơi chậm, nhưng để train thì chấp nhận được.
        # Hoặc tối ưu hơn: Ta chỉ train trên hàng, nhưng khi đánh giá ta tự xoay bàn cờ
        # bằng C++ (nhưng ở đây ta đang code Python).
        # Tạm thời ta chỉ dùng 4 hàng ngang để test cơ chế trước. 
        # (Lưu ý: Nếu chỉ dùng hàng ngang, AI sẽ chơi yếu ở cột dọc, nhưng ta sẽ nâng cấp sau).
        
        return score

    def update(self, board_state, delta, learning_rate):
        """
        Cập nhật trọng số (Học).
        delta: Sự khác biệt giữa giá trị ước tính và thực tế (Reward + V_next - V_curr).
        """
        # Cập nhật cho 4 hàng
        for i in range(0, 64, 16):
            row_idx = (board_state >> i) & 0xFFFF
            self.weights[row_idx] += learning_rate * delta

    def save(self, path):
        with open(path, 'wb') as f:
            pickle.dump(self.weights, f)
            
    def load(self, path):
        with open(path, 'rb') as f:
            self.weights = pickle.load(f)
