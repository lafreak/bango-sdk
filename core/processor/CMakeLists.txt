cmake_minimum_required(VERSION 3.5.2 FATAL_ERROR)

project(bangoprocessor)

set (CMAKE_CXX_STANDARD 11)

include_directories(include)

set(SOURCES
  src/db.cpp
  src/file.cpp
  src/parser.cpp
  src/lisp.cpp
  )

add_library(bangoprocessor ${SOURCES})
target_include_directories(bangoprocessor PUBLIC include)

IF(USE_GTEST)
  find_package(GTest REQUIRED)
  include_directories(${GTEST_INCLUDE_DIRS})
  add_executable(bangoprocessor_test test/main.cpp)
  target_link_libraries(bangoprocessor_test ${GTEST_LIBRARIES} bangoprocessor pthread)
ENDIF()

IF(USE_BENCHMARK)
  add_executable(bangoprocessor_benchmark benchmark/main.cpp)
  target_link_libraries(bangoprocessor_benchmark benchmark bangoprocessor pthread)
ENDIF()