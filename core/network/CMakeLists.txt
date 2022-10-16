cmake_minimum_required(VERSION 3.5.2 FATAL_ERROR)

project(bangonetwork)

set (CMAKE_CXX_STANDARD 14)

include_directories(include)
include_directories(include)

set(SOURCES
  src/authorizable.cpp
  src/packet.cpp
  src/writable.cpp
  src/client.cpp
  src/server.cpp
  )

add_library(bangonetwork ${SOURCES})
target_include_directories(bangonetwork PUBLIC include)

IF(USE_GTEST)
  find_package(GTest REQUIRED)
  include_directories(${GTEST_INCLUDE_DIRS})
  add_executable(bangonetwork_test test/main.cpp)
  target_link_libraries(bangonetwork_test ${GTEST_LIBRARIES} bangonetwork pthread)
ENDIF()

IF(USE_BENCHMARK)
  add_executable(bangonetwork_benchmark benchmark/main.cpp)
  target_link_libraries(bangonetwork_benchmark benchmark bangonetwork pthread)
ENDIF()