cmake_minimum_required(VERSION 3.31)
project(fpga_hft_engine)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
)
FetchContent_MakeAvailable(googletest)

# Main executable
add_executable(fpga_hft_engine src/main.cpp
        include/common/dummy.h
        include/logging/dummy2.h)

# Test executable
add_executable(tests
        include/common/dummy.h
        include/logging/dummy2.h
        tests/logger_test.cpp)
target_link_libraries(tests GTest::gtest_main)
