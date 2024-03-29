cmake_minimum_required(VERSION 3.5.2 FATAL_ERROR)

project(bangoprocessor)

set (CMAKE_CXX_STANDARD 17)

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
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  )
  # For Windows: Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  enable_testing()
  add_executable(
    bangoprocessor_test
    test/main.cpp
  )
  target_link_libraries(
    bangoprocessor_test
    GTest::gtest_main
    bangoprocessor
    pthread
  )
  include(GoogleTest)
  gtest_discover_tests(bangoprocessor_test)
ENDIF()

IF(USE_BENCHMARK)
  add_executable(bangoprocessor_benchmark benchmark/main.cpp)
  target_link_libraries(bangoprocessor_benchmark benchmark bangoprocessor pthread)
ENDIF()