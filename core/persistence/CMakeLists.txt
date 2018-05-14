cmake_minimum_required(VERSION 3.5.2 FATAL_ERROR)

project(bangopersistence)

set (CMAKE_CXX_STANDARD 11)

include_directories(include)
include_directories(include/zdb)

set(SOURCES
  src/connection.cpp
  src/pool.cpp
  src/query.cpp
  )

add_library(bangopersistence ${SOURCES})
target_include_directories(bangopersistence PUBLIC include include/zdb)