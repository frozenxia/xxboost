cmake_minimum_required (VERSION 2.8.0)

project (FastRGF)

# whether to use openmp (default is on)
option(OPENMP "Use openmp for multi-threads" ON)

set(CMAKE_CXX_FLAGS "-O3 -std=c++11")
#set(CMAKE_CXX_FLAGS "-g -std=c++11 -Wall")
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -ftree-vectorize -ffast-math")
endif()
get_directory_property(cDirDefs DIRECTORY ${CMAKE_SOURCE_DIR} COMPILE+DEFINITIONS)

include_directories(include)
add_subdirectory(src/base)
#add_subdirectory(src/forest)
add_subdirectory(src/exe)