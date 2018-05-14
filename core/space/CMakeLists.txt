cmake_minimum_required(VERSION 3.5.2 FATAL_ERROR)

project(bangospace)

set (CMAKE_CXX_STANDARD 11)

include_directories(include)

set(SOURCES
  src/map.cpp
  )

add_library(bangospace ${SOURCES})
target_include_directories(bangospace PUBLIC include)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
add_executable(bangospace_test test/main.cpp)
target_link_libraries(bangospace_test ${GTEST_LIBRARIES} bangospace pthread)