cmake_minimum_required(VERSION 3.31)
project(fpga_hft_engine)

set(CMAKE_CXX_STANDARD 20)

add_executable(fpga_hft_engine
        src/main.cpp
        src/Logger.cpp
)
