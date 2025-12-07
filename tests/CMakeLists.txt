include(FetchContent)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

FetchContent_MakeAvailable(googletest)

add_executable(unit_tests board-test.cpp)

target_link_libraries(unit_tests PRIVATE core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(unit_tests)