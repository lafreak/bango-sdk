cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(dbserver)

set (CMAKE_CXX_STANDARD 14)

include_directories(include)
include_directories(../shared)
include_directories(bangonetwork)
include_directories(bangopersistence)

include(FetchContent)
FetchContent_Declare(
  cli11
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11
  GIT_TAG        v2.2.0
)

FetchContent_MakeAvailable(cli11)
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
  CLI11
  )