cmake_minimum_required(VERSION 3.13)

project(scopefunctionscpp
VERSION 1.0
DESCRIPTION "Implementation of kotlin style scope functions in C++"
LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

set(HEADER_LIST ${CMAKE_CURRENT_SOURCE_DIR}/scopefn.hpp)

option(BUILD_TESTS OFF)
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

install(FILES
        ${HEADER_LIST}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include)