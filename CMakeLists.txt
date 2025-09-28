cmake_minimum_required(VERSION 3.31)
project(fpga_hft_engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use Release build by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Platform-specific compiler flags
if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -mtune=native")
endif()

include(FetchContent)

# Try the latest GoogleTest version with better C++20 support
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.15.2  # Latest version as of 2024
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Fetch nlohmann/json
FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG        v3.11.3
)
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(nlohmann_json)

# Force ARM64 OpenSSL path on macOS
if(APPLE)
    set(OPENSSL_ROOT_DIR "/opt/homebrew")
    set(OPENSSL_INCLUDE_DIR "/opt/homebrew/include")
    set(OPENSSL_CRYPTO_LIBRARY "/opt/homebrew/lib/libcrypto.dylib")
    set(OPENSSL_SSL_LIBRARY "/opt/homebrew/lib/libssl.dylib")
    set(CMAKE_PREFIX_PATH "/opt/homebrew" ${CMAKE_PREFIX_PATH})
    set(CMAKE_IGNORE_PATH "/usr/local/opt/openssl")
endif()

find_package(OpenSSL REQUIRED)

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
        src/client/BinanceClient.cpp
        include/client/BinanceClient.h
        src/Coin.cpp
        src/Logger.cpp
        include/logging/Logger.h
        include/common/Coin.h
        src/CoinManager.cpp
        include/common/CoinManager.h
        src/Visualizer.cpp
        include/common/Visualizer.h)

target_link_libraries(crypto_fpga_trader
        ixwebsocket
        nlohmann_json::nlohmann_json
)

# Test executable
add_executable(tests
        src/client/BinanceClient.cpp
        src/CoinManager.cpp
        src/Coin.cpp
        src/Logger.cpp
        src/Visualizer.cpp
        tests/TestLogger.cpp
        tests/TestBinanceClient.cpp
        tests/TestCoinManager.cpp
)

target_include_directories(tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
)

# Try the simple linking approach that worked in your original
target_link_libraries(tests
        ixwebsocket
        nlohmann_json::nlohmann_json
        GTest::gtest_main
        GTest::gmock_main
)

# Add macOS frameworks
if(APPLE)
    target_link_libraries(tests
            "-framework Security"
            "-framework Foundation"
    )
endif()