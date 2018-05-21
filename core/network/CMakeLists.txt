cmake_minimum_required(VERSION 3.5.2 FATAL_ERROR)

project(bangonetwork)

set (CMAKE_CXX_STANDARD 14)

include_directories(include)

set(SOURCES
  src/packet.cpp
  src/writable.cpp
  src/client.cpp
  src/server.cpp
  )

add_library(bangonetwork ${SOURCES})
target_include_directories(bangonetwork PUBLIC include)
