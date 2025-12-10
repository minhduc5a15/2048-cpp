#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // Automatically convert std::vector to Python List
#include "../core/board.h"

namespace py = pybind11;

// Wrapper to expose Enum Direction to Python
void init_enums(const py::module_& m) {
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
        .def(py::init<>()) // Default constructor
        .def("reset", &tfe::core::Board::reset)
        
        // Move function, returns true if board changed
        .def("move", &tfe::core::Board::move)
        
        .def("is_game_over", &tfe::core::Board::isGameOver)
        .def("get_score", &tfe::core::Board::getScore)
        
        // Return Grid (List[List[int]]) for UI rendering or debugging
        .def("get_grid", &tfe::core::Board::getGrid)
        
        // CRITICAL FOR AI: Get raw Bitboard (64-bit integer)
        // Much faster than get_grid as it avoids array copying
        .def("get_state", [](const tfe::core::Board& b) {
            return b.getState().board;
        })
        
        // Set state function (for AI to reload state - e.g., MCTS)
        .def("set_state", [](tfe::core::Board& b, const uint64_t board, const int score) {
            // Create a temporary GameState struct
            tfe::core::GameState state{};
            state.board = board;
            state.score = score;
            b.loadState(state);
        });
}