add_library(core STATIC board.cpp game-saver.cpp lookup_table.cpp ai_solver.cpp)
target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(core PRIVATE utils score nlohmann_json::nlohmann_json platform)