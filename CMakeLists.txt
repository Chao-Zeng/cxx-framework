cmake_minimum_required(VERSION 2.8)

project(cxx_framework)

#platform check
#include(CheckIncludeFile)
include(CheckSymbolExists)
#include(CheckTypeSize)

#check_include_file(unistd.h HAVE_UNISTD_H)

check_symbol_exists(daemon unistd.h HAVE_DAEMON)
if(NOT HAVE_DAEMON)
    message(FATAL_ERROR "require daemon symbol")
endif()

check_symbol_exists(strsignal string.h HAVE_STRSIGNAL)
if(NOT HAVE_STRSIGNAL)
    message(FATAL_ERROR "require strsignal symbol")
endif()

#version 1.0.0
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)

configure_file("${PROJECT_SOURCE_DIR}/version.h.in"
               "${PROJECT_BINARY_DIR}/version.h")
include_directories("${PROJECT_BINARY_DIR}")

#make VERBOSE=1
#set(CMAKE_VERBOSE_MAKEFILE ON)

#set compiler
#set(CMAKE_CXX_COMPILER "g++")

#Flags for all build types
set(CMAKE_CXX_FLAGS "-std=c++11")

#Flags for Debug build
set(CMAKE_CXX_FLAGS_DEBUG "-g")

#Flags for Release build type
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

#set default build type to Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

#Adds -D define flags
add_definitions(-DBOOST_LOG_DYN_LINK)

#Add include directories
#include_directories(${PROJECT_SOURCE_DIR}/3rdparty/boost/boost_1_63_0/include)
include_directories(${PROJECT_SOURCE_DIR}/src)

#Add link directories
#link_directories(${PROJECT_SOURCE_DIR}/3rdparty/boost/boost_1_63_0/lib)

#Add link libraries
#attention dependence sequence, less dependence put ahead
set(LIBS pthread boost_thread boost_system boost_log boost_log_setup boost_filesystem)

#Add source file directories
aux_source_directory(${PROJECT_SOURCE_DIR}/src SOURCES)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/log SOURCES)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/config SOURCES)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/utils SOURCES)

#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
#set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

#Add an executable
add_executable(cxx_framework ${SOURCES})

#Specify libraries or flags to use when linking a given target and/or its dependents
target_link_libraries(cxx_framework ${LIBS})

