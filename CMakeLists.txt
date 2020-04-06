cmake_minimum_required(VERSION 2.8)
project(live-glsl)

set(EXECUTABLE_NAME live-glsl.out)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -stdlib=libc++ -g -O0 -Wno-deprecated-declarations")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -stdlib=libc++ -O3 -Wno-deprecated-declarations")

include_directories(${CMAKE_SOURCE_DIR}/deps)
include_directories(${CMAKE_SOURCE_DIR}/deps/fontstash)
include_directories(${CMAKE_SOURCE_DIR}/deps/glm)
include_directories(${CMAKE_SOURCE_DIR}/deps/glm/gtc)
include_directories(${CMAKE_SOURCE_DIR}/deps/glfw/include)
include_directories(${CMAKE_SOURCE_DIR}/deps/glfw/deps)

add_subdirectory(${CMAKE_SOURCE_DIR}/deps/glm)

option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_BUILD_DOCS OFF)
option(GLFW_INSTALL OFF)

add_subdirectory(${CMAKE_SOURCE_DIR}/deps/glfw)

add_executable(${EXECUTABLE_NAME} 
	${CMAKE_SOURCE_DIR}/src/main.cpp
	${CMAKE_SOURCE_DIR}/src/gui.cpp
	${CMAKE_SOURCE_DIR}/deps/glfw/deps/glad.c)

target_link_libraries(${EXECUTABLE_NAME} glfw ${GLFW_LIBRARIES})