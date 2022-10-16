cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

project(debugapp)

set (CMAKE_CXX_STANDARD 14)

include_directories(bangonetwork)

set(SOURCES
  main.cpp
  )
add_executable(debugapp ${SOURCES})
target_link_libraries(debugapp 
  bangonetwork 
  tacopie 
  pthread
  )