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
    bangoutils_test
    test/main.cpp
  )
  target_link_libraries(
    bangoutils_test
    GTest::gtest_main
    bangoutils
    pthread
  )
  include(GoogleTest)
  gtest_discover_tests(bangoutils_test)
ENDIF()

IF(USE_BENCHMARK)
  add_executable(bangoutils_benchmark benchmark/main.cpp)
  target_link_libraries(bangoutils_benchmark benchmark bangoutils pthread)
ENDIF()