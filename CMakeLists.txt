cmake_minimum_required(VERSION 3.15)

project(test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Werror=return-type)

add_executable(test test.cpp)