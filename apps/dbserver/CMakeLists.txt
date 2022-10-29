cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(dbserver)

set (CMAKE_CXX_STANDARD 14)

include_directories(include)
include_directories(../shared)
include_directories(bangonetwork)
include_directories(bangopersistence)

set(ZDB_ROOT ${CMAKE_BINARY_DIR}/thirdparty/zdb)
set(ZDB_LIB_DIR ${ZDB_ROOT}/bin/lib)
set(ZDB_INCLUDE_DIR ${ZDB_ROOT}/bin/include)

include(ExternalProject)
ExternalProject_Add(zdb_external
                    URL "https://www.tildeslash.com/libzdb/dist/libzdb-3.2.3.tar.gz"
                    CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${ZDB_ROOT}/bin"
                    BINARY_DIR ${ZDB_ROOT}/src/zdb
                    SOURCE_DIR ${ZDB_ROOT}/src/zdb
                    CONFIGURE_COMMAND ${ZDB_ROOT}/src/zdb/configure --prefix=${ZDB_ROOT}/bin
                    BUILD_COMMAND make
                    INSTALL_COMMAND make install
                    BUILD_BYPRODUCTS ${ZDB_LIB_DIR}/libzdb.a)
link_directories(${ZDB_LIB_DIR})

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