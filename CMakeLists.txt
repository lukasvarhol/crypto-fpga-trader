cmake_minimum_required(VERSION 3.31)
project(fpga_hft_engine)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -mtune=native")

# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
)
FetchContent_MakeAvailable(googletest)

# Fetch Google Benchmark
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

# Benchmarks executable
add_executable(benchmark_runner benchmarks/BenchmarkLogger.cpp)
target_link_libraries(benchmark_runner benchmark::benchmark)

# Add custom target to build and run benchmarks in one command
add_custom_target(run_benchmarks
        COMMAND benchmark_runner
        DEPENDS benchmark_runner
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Building and running benchmarks..."
)