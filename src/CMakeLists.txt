add_subdirectory(utils)
add_subdirectory(core)
add_subdirectory(renderer) # Console renderer
add_subdirectory(input)
add_subdirectory(game)     # Console game loop
add_subdirectory(gui)      # GUI module

add_executable(2048-game main.cpp)
target_link_libraries(2048-game PRIVATE game)

add_executable(2048-gui main-gui.cpp)
target_link_libraries(2048-gui PRIVATE gui)