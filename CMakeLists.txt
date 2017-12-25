cmake_minimum_required(VERSION 2.8)
project(live-glsl)

set(EXECUTABLE_NAME live-glsl.out)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -stdlib=libc++ -03")

file(GLOB SOURCES src/*.cpp)

include_directories(${CMAKE_SOURCE_DIR}/lib/include/fontstash)

add_subdirectory(${CMAKE_SOURCE_DIR}/lib/include/glm)
include_directories(${CMAKE_SOURCE_DIR}/lib/include/glm)
include_directories(${CMAKE_SOURCE_DIR}/lib/include/glm/gtc)

option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_BUILD_DOCS OFF)
option(GLFW_INSTALL OFF)

add_subdirectory(${CMAKE_SOURCE_DIR}/deps/glfw)
include_directories(${CMAKE_SOURCE_DIR}/deps/glfw/include)

add_executable(${EXECUTABLE_NAME} ${SOURCES})

target_link_libraries(${EXECUTABLE_NAME} glfw ${GLFW_LIBRARIES})
