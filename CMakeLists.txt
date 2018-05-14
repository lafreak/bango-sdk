cmake_minimum_required(VERSION 3.5.2 FATAL_ERROR)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

add_subdirectory(core/network)
add_subdirectory(core/persistence)
add_subdirectory(apps/dbserver)
add_subdirectory(apps/gameserver)