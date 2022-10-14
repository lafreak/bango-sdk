cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

project(bango-sdk)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

option(USE_GTEST "GTEST" OFF)
option(USE_BENCHMARK "BENCHMARK" OFF)

set(TACOPIE_ROOT ${CMAKE_BINARY_DIR}/thirdparty/tacopie)
set(TACOPIE_LIB_DIR ${TACOPIE_ROOT}/bin/lib)
set(TACOPIE_INCLUDE_DIR ${TACOPIE_ROOT}/bin/include)

include(ExternalProject)
ExternalProject_Add(tacopie_external
                    PREFIX ${TACOPIE_ROOT}
                    GIT_REPOSITORY "https://github.com/lafreak/tacopie.git"
                    GIT_TAG "1ef58161e23f3ff6d4c57703ead349bf9c61d0d5"
                    UPDATE_COMMAND ""
                    PATCH_COMMAND ""
                    CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${TACOPIE_ROOT}/bin"
                    BINARY_DIR ${TACOPIE_ROOT}/src/tacopie
                    SOURCE_DIR ${TACOPIE_ROOT}/src/tacopie
                    INSTALL_DIR ${TACOPIE_ROOT}/bin
                    BUILD_COMMAND make
                    BUILD_BYPRODUCTS ${TACOPIE_LIB_DIR}/libtacopie.a)
link_directories(${TACOPIE_LIB_DIR})

add_subdirectory(core/network)
add_subdirectory(core/persistence)
add_subdirectory(core/space)
add_subdirectory(core/processor)
add_subdirectory(core/utils)
add_subdirectory(apps/dbserver)
add_subdirectory(apps/gameserver)
add_subdirectory(apps/debugapp)
