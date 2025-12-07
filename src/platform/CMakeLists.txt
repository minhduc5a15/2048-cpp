add_library(platform STATIC platform.cpp)

target_include_directories(platform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
