cmake_minimum_required(VERSION 3.5.2 FATAL_ERROR)

project(gameserver)

set (CMAKE_CXX_STANDARD 14)

include_directories(include)
include_directories(../shared)
include_directories(bangonetwork)
include_directories(bangospace)
include_directories(bangoprocessor)
include_directories(bangoutils)

link_directories(../../thirdparty)

set(SOURCES
  src/CommandDispatcher.cpp
  src/User.cpp
  src/Character.cpp
  src/NPC.cpp
  src/Player.cpp
  src/Monster.cpp
  src/Socket.cpp
  src/World.cpp
  src/Inventory.cpp
  src/DBListener.cpp
  src/main.cpp
  )

add_executable(gameserver ${SOURCES})
target_link_libraries(gameserver 
  bangonetwork 
  bangospace 
  bangoprocessor 
  bangoutils 
  tacopie 
  pthread
  )
