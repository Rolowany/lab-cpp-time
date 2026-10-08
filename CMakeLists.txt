cmake_minimum_required(VERSION 4.3)
project(time_lab)

set(CMAKE_CXX_STANDARD 20)

add_executable(ctime examples/ctime.cpp)
add_executable(arithmetic examples/arithmetic_chrono.cpp)
add_executable(benchmarking examples/benchmarking.cpp)
add_executable(calendar examples/calendar.cpp)