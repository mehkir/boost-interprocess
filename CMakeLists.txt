cmake_minimum_required(VERSION 3.22.1)
project(boost-interprocess)

set(CMAKE_CXX_STANDARD 23)

add_executable(boost-interprocess boost-interprocess.cpp)
target_include_directories(boost-interprocess PUBLIC ${PROJECT_BINARY_DIR})
target_link_libraries(boost-interprocess boost_system)