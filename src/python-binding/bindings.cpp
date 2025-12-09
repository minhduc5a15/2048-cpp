#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // Tự động chuyển đổi std::vector sang Python List
#include "../core/board.h"

namespace py = pybind11;

// Wrapper để lộ Enum Direction cho Python
void init_enums(py::module_& m) {
    py::enum_<tfe::core::Direction>(m, "Direction")
        .value("Up", tfe::core::Direction::Up)
        .value("Down", tfe::core::Direction::Down)
        .value("Left", tfe::core::Direction::Left)
        .value("Right", tfe::core::Direction::Right)
        .export_values();
}

PYBIND11_MODULE(py2048, m) {
    m.doc() = "2048 Core C++ Optimized using Bitboard for AI Training";

    init_enums(m);

    py::class_<tfe::core::Board>(m, "Board")
        .def(py::init<>()) // Constructor mặc định
        .def("reset", &tfe::core::Board::reset)
        
        // Hàm di chuyển, trả về true nếu bàn cờ thay đổi
        .def("move", &tfe::core::Board::move)
        
        .def("is_game_over", &tfe::core::Board::isGameOver)
        .def("get_score", &tfe::core::Board::getScore)
        
        // Trả về Grid (List[List[int]]) để vẽ UI hoặc debug
        .def("get_grid", &tfe::core::Board::getGrid)
        
        // QUAN TRỌNG CHO AI: Lấy trực tiếp Bitboard (số nguyên 64-bit)
        // Việc này nhanh hơn nhiều so với get_grid vì không phải copy mảng
        .def("get_state", [](const tfe::core::Board& b) {
            return b.getState().board;
        })
        
        // Hàm set state (để AI load lại trạng thái nếu cần - ví dụ MCTS)
        .def("set_state", [](tfe::core::Board& b, uint64_t board, int score) {
            // Chúng ta cần tạo struct GameState tạm
            tfe::core::GameState state;
            state.board = board;
            state.score = score;
            b.loadState(state);
        });
}
