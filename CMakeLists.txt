cmake_minimum_required(VERSION 3.1)
project(trans-perf-test)

# The version number.
set (TRANS_PERF_TEST_VERSION_MAJOR 0)
set (TRANS_PERF_TEST_VERSION_MAJOR 1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")

add_subdirectory(src)
add_subdirectory(scripts)