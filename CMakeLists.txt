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

include(FetchContent)
FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG        v1.8.3
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(benchmark)

# Main executable
add_executable(fpga_hft_engine src/main.cpp
        include/common/dummy.h
        include/logging/dummy2.h)

# Test executable
add_executable(tests tests/TestLogger.cpp
        include/common/dummy.h
        include/logging/dummy2.h
        # src/logger.cpp
)
target_link_libraries(tests GTest::gtest_main GTest::gmock_main)

add_executable(benchmarks benchmarks/BenchmarkLogger.cpp)
target_link_libraries(benchmarks benchmark::benchmark)