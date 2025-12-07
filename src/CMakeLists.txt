add_subdirectory(utils)
add_subdirectory(core)
add_subdirectory(renderer)
add_subdirectory(input)
add_subdirectory(game)

add_executable(2048-game main.cpp)

target_link_libraries(2048-game PRIVATE game)