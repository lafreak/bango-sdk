cmake_minimum_required(VERSION 3.5.2 FATAL_ERROR)

project(bangoutils)

set (CMAKE_CXX_STANDARD 11)

include_directories(include)

set(SOURCES
  src/random.cpp
  src/time.cpp
  )

add_library(bangoutils ${SOURCES})
target_include_directories(bangoutils PUBLIC include)

IF(USE_GTEST)
  find_package(GTest REQUIRED)
  include_directories(${GTEST_INCLUDE_DIRS})
  add_executable(bangoutils_test test/main.cpp)
  target_link_libraries(bangoutils_test ${GTEST_LIBRARIES} bangoutils pthread)
ENDIF()

IF(USE_BENCHMARK)
  add_executable(bangoutils_benchmark benchmark/main.cpp)
  target_link_libraries(bangoutils_benchmark benchmark bangoutils pthread)
ENDIF()