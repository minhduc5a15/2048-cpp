add_library(renderer STATIC console-renderer.cpp)
target_include_directories(renderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(renderer PRIVATE core)