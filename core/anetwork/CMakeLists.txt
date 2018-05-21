cmake_minimum_required(VERSION 3.5.2 FATAL_ERROR)

project(bangoanetwork)

set (CMAKE_CXX_STANDARD 14)

include_directories(include)

set(SOURCES
  src/packet.cpp
  src/writable.cpp
  src/client.cpp
  src/server.cpp
  )

add_library(bangoanetwork ${SOURCES})
target_include_directories(bangoanetwork PUBLIC include)
