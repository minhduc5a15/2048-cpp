add_library(gui STATIC
        raylib-renderer.cpp
        gui-game.cpp
)

target_include_directories(gui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(gui PRIVATE raylib score PUBLIC core ai)