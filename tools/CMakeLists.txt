add_executable(gen_baseline gen_baseline.cpp)
target_link_libraries(gen_baseline PRIVATE core utils)
target_include_directories(gen_baseline PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(bench_bitboard_ops bench_bitboard_ops.cpp)
target_link_libraries(bench_bitboard_ops PRIVATE core utils)
target_include_directories(bench_bitboard_ops PRIVATE ${CMAKE_SOURCE_DIR}/src)
