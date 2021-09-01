cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(logger VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

set (CMAKE_CXX_FLAGS "-std=c++17 -g2 -D_DEBUG=1 -Wall -Wextra -Wformat-security -Wduplicated-cond -Wfloat-equal -Wshadow -Wconversion -Wlogical-not-parentheses -Wnull-dereference -pthread")

# add the executable
add_executable(logger logger.cpp loggerapp.cpp)


