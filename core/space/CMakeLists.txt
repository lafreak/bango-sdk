cmake_minimum_required(VERSION 3.5.2 FATAL_ERROR)

project(bangospace)

set (CMAKE_CXX_STANDARD 14)

include_directories(include)

set(SOURCES
  src/quadtree.cpp
  )

add_library(bangospace ${SOURCES})
target_include_directories(bangospace PUBLIC include)

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
    bangospace_test
    test/main.cpp
  )
  target_link_libraries(
    bangospace_test
    GTest::gtest_main
    GTest::gmock_main
    bangospace
    pthread
  )
  include(GoogleTest)
  gtest_discover_tests(bangospace_test)
ENDIF()

IF(USE_BENCHMARK)
  add_executable(bangospace_benchmark benchmark/main.cpp)
  target_link_libraries(bangospace_benchmark benchmark bangospace pthread)
ENDIF()