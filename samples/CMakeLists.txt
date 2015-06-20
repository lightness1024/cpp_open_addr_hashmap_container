cmake_minimum_required (VERSION 2.6)

project (oahm)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
 add_definitions(-std=c++11 -D_DEBUG -O0 -g)
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Release")
 add_definitions(-std=c++11 -DNDEBUG -O3)
endif()

include_directories("${PROJECT_BINARY_DIR}/../src")

add_executable(oahm example.cpp)
