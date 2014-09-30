cmake_minimum_required(VERSION 2.8)
project(shadertool)

set(EXECUTABLE_NAME shadertool.out)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/build/modules) 
set(CMAKE_CXX_FLAGS "-Wall -std=c++0x -stdlib=libc++ -I/usr/include")
set(CXX_FLAGS_DEBUG "${CXX_FLAGS_DEBUG} -g -O0")

set(SOURCES src/main.cpp)

add_subdirectory(${CMAKE_SOURCE_DIR}/ogl)

#add_executable(${EXECUTABLE_NAME} ${SOURCES} ogl)