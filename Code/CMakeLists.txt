cmake_minimum_required(VERSION 3.10)
project(Code)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/inc)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB_RECURSE HEADERS "${INC_DIR}/*.h")
file(GLOB_RECURSE SOURCES "${SRC_DIR}/*.cpp")

include_directories(${INC_DIR})

add_executable(Code main.cpp ${SOURCES} ${HEADERS})
