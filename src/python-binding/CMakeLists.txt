# Tạo module python tên là py2048
pybind11_add_module(py2048 bindings.cpp)

# Link với thư viện core của chúng ta
target_link_libraries(py2048 PRIVATE core)

# Tắt tiếp tố phiên bản python để tên file gọn hơn (tùy chọn, nhưng tiện cho dev)
set_target_properties(py2048 PROPERTIES OUTPUT_NAME "py2048")

# Copy file thư viện sau khi build ra thư mục gốc để dễ import test
# (Trên Linux sẽ là file py2048.cpython-310-x86_64-linux-gnu.so)
add_custom_command(TARGET py2048 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:py2048> ${CMAKE_SOURCE_DIR}/
)
