cmake_minimum_required(VERSION 3.10)
project(log_test)

set(CMAKE_CXX_STANDARD 17)

find_package(spdlog REQUIRED)
add_executable(log_test test.cpp)
target_link_libraries(log_test PRIVATE spdlog::spdlog)