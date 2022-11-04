cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(bangonetwork)

set (CMAKE_CXX_STANDARD 14)

include_directories(include)

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


set(SOURCES
  src/authorizable.cpp
  src/packet.cpp
  src/writable.cpp
  src/client.cpp
  src/server.cpp
  )

add_library(bangonetwork ${SOURCES})
target_include_directories(bangonetwork PUBLIC include ${TACOPIE_INCLUDE_DIR})
target_link_directories(bangonetwork PUBLIC ${TACOPIE_LIB_DIR})

IF(USE_GTEST)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  )
  # For Windows: Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  enable_testing()
  add_executable(
    bangonetwork_test
    test/main.cpp
  )
  target_link_libraries(
    bangonetwork_test
    GTest::gtest_main
    bangonetwork
    tacopie
    pthread
  )
  include(GoogleTest)
  gtest_discover_tests(bangonetwork_test)
ENDIF()

IF(USE_BENCHMARK)
  add_executable(bangonetwork_benchmark benchmark/main.cpp)
  target_link_libraries(bangonetwork_benchmark benchmark bangonetwork tacopie pthread)
ENDIF()