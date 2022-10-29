cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

project(bango-sdk)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

option(USE_GTEST "GTEST" OFF)
option(USE_BENCHMARK "BENCHMARK" OFF)

add_subdirectory(core/network)
add_subdirectory(core/persistence)
add_subdirectory(core/space)
add_subdirectory(core/processor)
add_subdirectory(core/utils)
add_subdirectory(apps/dbserver)
add_subdirectory(apps/gameserver)
add_subdirectory(apps/debugapp)
