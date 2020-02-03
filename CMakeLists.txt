cmake_minimum_required(VERSION 3.5.0)

project (retro8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/projects/cmake")

option(OPENDINGUX "Build on opendingux toolchain" OFF)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  set(CMAKE_BUILD_TYPE "Debug")
endif()

if(OPENDINGUX)
  set(CMAKE_CXX_COMPILER "$ENV{CROSS}g++" CACHE PATH "" FORCE)
  set(CMAKE_C_COMPILER "$ENV{CROSS}gcc" CACHE PATH "" FORCE)
  set(CMAKE_SYSROOT "/opt/gcw0-toolchain/usr/mipsel-gcw0-linux-uclibc/sysroot")
endif()

find_package(SDL2 REQUIRED)

add_compile_options(-Wno-unused-parameter -Wno-missing-field-initializers
  -Wno-sign-compare -Wno-parentheses -Wno-unused-variable
)

add_compile_options(-g -O2 -W -Wall -Wextra)
add_compile_options(
  $<$<COMPILE_LANGUAGE:CXX>:-Wno-reorder>
)

if(APPLE)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++14>)
else()
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++11>)
endif()

include_directories(src)
include_directories(${SDL2_INCLUDE_DIR})

set(SRC_ROOT "${CMAKE_SOURCE_DIR}/src")

file(GLOB SOURCES_ROOT "${SRC_ROOT}/*.cpp")
file(GLOB SOURCES_VIEWS "${SRC_ROOT}/views/*.cpp")
file(GLOB SOURCES_IO "${SRC_ROOT}/io/*.cpp")
file(GLOB SOURCES_VM "${SRC_ROOT}/vm/*.cpp")
file(GLOB SOURCES_LUA "${SRC_ROOT}/lua/*.c")

set(SOURCES ${SOURCES_ROOT} ${SOURCES_VIEWS} ${SOURCES_IO} ${SOURCES_VM} ${SOURCES_LUA})

add_executable(retro8 ${SOURCES})

target_link_libraries(retro8 ${SDL2_LIBRARY})
