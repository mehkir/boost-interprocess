cmake_minimum_required(VERSION 3.22.1)
project(boost-interprocess)

set(CMAKE_CXX_STANDARD 23)

add_executable(boost-interprocess-example boost-interprocess-example.cpp)
add_executable(boost-interprocess-custom boost-interprocess-custom.cpp)