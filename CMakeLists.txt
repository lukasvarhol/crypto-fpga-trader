cmake_minimum_required(VERSION 3.31)
project(fpga_hft_engine)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)

# Platform-specific compiler flags
if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -mtune=native")
endif()

# Fetch all dependencies via FetchContent (no system dependencies)
include(FetchContent)

# Fetch GoogleTest
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

# Fetch nlohmann/json
FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG        v3.11.3
)
set(JSON_BuildTests OFF CACHE INTERNAL "")  # Don't build json tests
set(JSON_Install OFF CACHE INTERNAL "")     # Don't install json
FetchContent_MakeAvailable(nlohmann_json)

# Force ARM64 OpenSSL path on macOS and ignore Intel version
if(APPLE)
    set(OPENSSL_ROOT_DIR "/opt/homebrew")
    set(OPENSSL_INCLUDE_DIR "/opt/homebrew/include")
    set(OPENSSL_CRYPTO_LIBRARY "/opt/homebrew/lib/libcrypto.dylib")
    set(OPENSSL_SSL_LIBRARY "/opt/homebrew/lib/libssl.dylib")
    set(CMAKE_PREFIX_PATH "/opt/homebrew" ${CMAKE_PREFIX_PATH})

    # Explicitly ignore Intel OpenSSL
    set(CMAKE_IGNORE_PATH "/usr/local/opt/openssl")
endif()

find_package(OpenSSL REQUIRED)

# Verify we found the right OpenSSL
if(APPLE AND OPENSSL_FOUND)
    message(STATUS "OpenSSL SSL library: ${OPENSSL_SSL_LIBRARY}")
    message(STATUS "OpenSSL Crypto library: ${OPENSSL_CRYPTO_LIBRARY}")
endif()

# Fetch IXWebSocket
FetchContent_Declare(
        ixwebsocket
        GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
        GIT_TAG        v11.4.5
)
set(USE_TLS ON CACHE BOOL "" FORCE)
set(USE_ZLIB ON CACHE BOOL "" FORCE)
set(USE_OPEN_SSL ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(ixwebsocket)

# Main executable
add_executable(crypto_fpga_trader src/main.cpp
        include/common/dummy.h
        include/logging/dummy2.h
        src/client/BinanceClient.cpp
        include/client/BinanceClient.h)

target_link_libraries(crypto_fpga_trader
        ixwebsocket
        nlohmann_json::nlohmann_json
)

# Test executable
add_executable(tests tests/TestLogger.cpp
        include/common/dummy.h
        include/logging/dummy2.h)

target_link_libraries(tests
        GTest::gtest_main
        GTest::gmock_main
        nlohmann_json::nlohmann_json
)

# Benchmarks executable
add_executable(benchmark_runner benchmarks/BenchmarkLogger.cpp)
target_link_libraries(benchmark_runner
        benchmark::benchmark
        nlohmann_json::nlohmann_json
)

# Add custom target to build and run benchmarks in one command
add_custom_target(run_benchmarks
        COMMAND benchmark_runner
        DEPENDS benchmark_runner
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Building and running benchmarks..."
)