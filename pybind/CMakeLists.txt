pybind11_add_module(py2048 bindings.cpp)

target_link_libraries(py2048 PRIVATE core utils)

target_include_directories(py2048 PRIVATE ${CMAKE_SOURCE_DIR}/src)

if (NOT CMAKE_BUILD_TYPE MATCHES Debug)
    set_target_properties(py2048 PROPERTIES LTO ON)
endif ()

add_custom_command(TARGET py2048 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/python/gym2048/
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:py2048> ${CMAKE_SOURCE_DIR}/python/gym2048/
        COMMENT "Copying py2048 extension to python/gym2048 directory..."
)