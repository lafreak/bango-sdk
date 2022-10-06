cmake_minimum_required(VERSION 3.5.2 FATAL_ERROR)

project(dbserver)

set (CMAKE_CXX_STANDARD 14)

include_directories(include)
include_directories(../shared)
include_directories(bangonetwork)
include_directories(bangopersistence)

set(SOURCES
  src/DatabaseManager.cpp
  src/main.cpp
  )

add_executable(dbserver ${SOURCES})
target_link_libraries(dbserver 
  bangonetwork 
  bangopersistence 
  tacopie 
  zdb 
  mysqlclient
  pthread
  )