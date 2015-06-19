cmake_minimum_required (VERSION 2.6)
project (oahm)
include_directories("${PROJECT_BINARY_DIR}/../src")
add_executable(oahm example.cpp)
