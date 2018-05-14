cmake_minimum_required(VERSION 3.5.2 FATAL_ERROR)

project(gameserver)

set (CMAKE_CXX_STANDARD 11)

include_directories(include)
include_directories(../shared)
include_directories(bangonetwork)

link_directories(../../thirdparty)

set(SOURCES
  src/GameManager.cpp
  src/Inventory.cpp
  src/Item.cpp
  src/Player.cpp
  src/main.cpp
  )

add_executable(gameserver ${SOURCES})
target_link_libraries(gameserver 
  bangonetwork 
  tacopie 
  pthread
  )