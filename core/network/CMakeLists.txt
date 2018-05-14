cmake_minimum_required(VERSION 3.5.2 FATAL_ERROR)

project(bangonetwork)

set (CMAKE_CXX_STANDARD 11)

include_directories(include)

set(SOURCES
  src/component.cpp
  src/packet.cpp
  src/session.cpp
  src/tcp_client.cpp
  src/tcp_server.cpp
  )

add_library(bangonetwork ${SOURCES})
target_include_directories(bangonetwork PUBLIC include)