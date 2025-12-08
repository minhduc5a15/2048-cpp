#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // Tự động chuyển đổi std::vector <-> Python List

// Include các header file từ source gốc
#include "core/board.h"
#include "utils/random-generator.h"

namespace py = pybind11;
using namespace tfe::core;

PYBIND11_MODULE(py2048, m) {
    m.doc() = "2048 C++ Core exposed to Python via Pybind11";

    // 1. Bind Enum Direction
    py::enum_<Direction>(m, "Direction")
        .value("Up", Direction::Up)
        .value("Down", Direction::Down)
        .value("Left", Direction::Left)
        .value("Right", Direction::Right)
        .export_values();

    // 2. Bind Class Board
    py::class_<Board>(m, "Board")
        .def(py::init<int>(), py::arg("size") = 4)

        .def("reset", &Board::reset, "Reset the board to initial state")

        // Hàm move trả về bool (true nếu bàn cờ có thay đổi)
        .def("move", &Board::move, py::arg("direction"), "Execute a move")

        .def("get_grid", &Board::getGrid, "Get the board state as a 2D list")

        .def("get_score", &Board::getScore, "Get current score")

        .def("is_game_over", &Board::isGameOver, "Check if game is over");

    // 3. Bind chức năng Random Seed (quan trọng để train AI ổn định)
    m.def("set_seed", &tfe::utils::RandomGenerator::setSeed, "Set random seed");
}