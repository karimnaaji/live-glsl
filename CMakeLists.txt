cmake_minimum_required(VERSION 2.8)
project(shadertool)

set(EXECUTABLE_NAME shadertool.out)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/build/modules)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++0x -stdlib=libc++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/include")
set(CXX_FLAGS_DEBUG "${CXX_FLAGS_DEBUG} -g -O0")

set(SOURCES
    src/main.cpp
    src/filewatcher.cpp
    )

add_subdirectory(${CMAKE_SOURCE_DIR}/ogl)
include_directories(${CMAKE_SOURCE_DIR}/ogl/include)
include_directories(${CMAKE_SOURCE_DIR}/ogl/deps/math/include)

option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_BUILD_DOCS OFF)
option(GLFW_INSTALL OFF)

add_subdirectory(${CMAKE_SOURCE_DIR}/deps/glfw)
include_directories(${CMAKE_SOURCE_DIR}/deps/glfw/include)

add_executable(${EXECUTABLE_NAME} ${SOURCES})

target_link_libraries(${EXECUTABLE_NAME} ogl glfw ${GLFW_LIBRARIES})
