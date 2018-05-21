cmake_minimum_required(VERSION 3.5.2 FATAL_ERROR)

project(gameserver)

set (CMAKE_CXX_STANDARD 14)

include_directories(include)
include_directories(../shared)
include_directories(bangonetwork)
include_directories(bangospace)

link_directories(../../thirdparty)

set(SOURCES
  src/GameManager.cpp
  src/Inventory.cpp
  src/Item.cpp
  src/Player.cpp
  src/WorldMap.cpp
  src/main.cpp
  )

add_executable(gameserver ${SOURCES})
target_link_libraries(gameserver 
  bangonetwork 
  bangospace 
  tacopie 
  pthread
  )

IF(USE_GTEST)
  find_package(GTest REQUIRED)
  include_directories(${GTEST_INCLUDE_DIRS})
  add_executable(gameserver_test test/main.cpp)
  target_link_libraries(gameserver_test ${GTEST_LIBRARIES} bangospace pthread)
ENDIF()