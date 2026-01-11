add_subdirectory(utils)
add_subdirectory(platform)
add_subdirectory(score)
add_subdirectory(core)
add_subdirectory(ai)
add_subdirectory(renderer) # Console renderer
add_subdirectory(input)
add_subdirectory(game)     # Console game loop
add_subdirectory(gui)      # GUI module

add_executable(2048-game main.cpp)
target_link_libraries(2048-game PRIVATE game)

add_executable(2048-gui main-gui.cpp)
target_include_directories(2048-gui PUBLIC ${CMAKE_SOURCE_DIR}/core)
target_link_libraries(2048-gui PUBLIC gui)