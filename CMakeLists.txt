cmake_minimum_required(VERSION 2.8)

set(TARGET_PROGRAM_NAME cxx_framework)

project(${TARGET_PROGRAM_NAME})

#third party libraries
include(ExternalProject)

#boost
set(BOOST_DOWNLOAD_URL https://dl.bintray.com/boostorg/release/1.68.0/source/boost_1_68_0.tar.gz)
set(BOOST_DOWNLOAD_FILE_HASH SHA256=da3411ea45622579d419bfda66f45cd0f8c32a181d84adfa936f5688388995cf)
set(Boost_VERSION 1.68.0)

set(BOOST_ROOT ${PROJECT_SOURCE_DIR}/third_party/boost)
#didn't search system library
set(Boost_NO_SYSTEM_PATHS ON)

find_package(Boost ${Boost_VERSION})
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
else(Boost_FOUND)
    message(STATUS "Build boost ${Boost_VERSION}")
    ExternalProject_Add(Boost
        #directory options
        PREFIX ${BOOST_ROOT}

        #download options
        URL ${BOOST_DOWNLOAD_URL}
        URL_HASH ${BOOST_DOWNLOAD_FILE_HASH}

        #Configure options
        CONFIGURE_COMMAND ./bootstrap.sh --prefix=${BOOST_ROOT}

        #build options
        BUILD_COMMAND ./b2 -j8 --build-dir=./build toolset=gcc variant=release link=shared threading=multi cxxflags=-std=c++11 install
        BUILD_IN_SOURCE TRUE

        #install options
        INSTALL_COMMAND ""
        )
    include_directories(${BOOST_ROOT}/include)
    link_directories(${BOOST_ROOT}/lib)
endif(Boost_FOUND)


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
include_directories(${PROJECT_SOURCE_DIR}/src)

#Add link directories
#link_directories(path/to/lib)

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
add_executable(${TARGET_PROGRAM_NAME} ${SOURCES})

#Specify libraries or flags to use when linking a given target and/or its dependents
target_link_libraries(${TARGET_PROGRAM_NAME} ${LIBS})


#install
set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR})

install(TARGETS ${TARGET_PROGRAM_NAME}
    RUNTIME DESTINATION bin
    )

install(FILES
    "${PROJECT_SOURCE_DIR}/config/config.info"
    DESTINATION config
    )

install(FILES
    "third_party/boost/lib/libboost_filesystem.so"
    "third_party/boost/lib/libboost_filesystem.so.1.68.0"
    "third_party/boost/lib/libboost_log.so"
    "third_party/boost/lib/libboost_log.so.1.68.0"
    "third_party/boost/lib/libboost_log_setup.so"
    "third_party/boost/lib/libboost_log_setup.so.1.68.0"
    "third_party/boost/lib/libboost_regex.so"
    "third_party/boost/lib/libboost_regex.so.1.68.0"
    "third_party/boost/lib/libboost_system.so"
    "third_party/boost/lib/libboost_system.so.1.68.0"
    "third_party/boost/lib/libboost_thread.so"
    "third_party/boost/lib/libboost_thread.so.1.68.0"

    DESTINATION lib
    )

