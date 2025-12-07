add_library(score STATIC score-manager.cpp)

target_include_directories(score PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link privately, as only the implementation of the score library uses nlohmann_json.
# This provides the necessary include paths for compiling score-manager.cpp.
target_link_libraries(score PRIVATE nlohmann_json::nlohmann_json platform)
