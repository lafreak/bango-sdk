cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(bangopersistence)

set (CMAKE_CXX_STANDARD 11)

include_directories(include)

set(ZDB_ROOT ${CMAKE_BINARY_DIR}/thirdparty/zdb)
set(ZDB_LIB_DIR ${ZDB_ROOT}/bin/lib)
set(ZDB_INCLUDE_DIR ${ZDB_ROOT}/bin/include/zdb)
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


set(SOURCES
  src/connection.cpp
  src/pool.cpp
  src/query.cpp
  )

add_library(bangopersistence ${SOURCES})
target_include_directories(bangopersistence PUBLIC include ${ZDB_INCLUDE_DIR})
target_link_libraries(bangopersistence zdb)