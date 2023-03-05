cmake_minimum_required(VERSION 3.10)
project(live-glsl)

set(EXECUTABLE_NAME live-glsl)
set(CMAKE_CXX_STANDARD 11)

if (NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wno-unused-result -Wno-deprecated-declarations")
endif()

include_directories(${CMAKE_SOURCE_DIR}/deps)
include_directories(${CMAKE_SOURCE_DIR}/deps/fontstash)
include_directories(${CMAKE_SOURCE_DIR}/deps/glm)
include_directories(${CMAKE_SOURCE_DIR}/deps/glm/gtc)
include_directories(${CMAKE_SOURCE_DIR}/deps/glfw/include)
include_directories(${CMAKE_SOURCE_DIR}/deps/glfw/deps)
include_directories(${CMAKE_SOURCE_DIR}/deps/getopt/include)
include_directories(${CMAKE_SOURCE_DIR}/tests)
include_directories(${CMAKE_SOURCE_DIR}/src)

add_subdirectory(${CMAKE_SOURCE_DIR}/deps/glm)

option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_BUILD_DOCS OFF)
option(GLFW_INSTALL OFF)

add_subdirectory(${CMAKE_SOURCE_DIR}/deps/glfw)

set(SOURCES
	${CMAKE_SOURCE_DIR}/src/gui.cpp
	${CMAKE_SOURCE_DIR}/src/filewatcher.cpp
	${CMAKE_SOURCE_DIR}/src/shader.cpp
	${CMAKE_SOURCE_DIR}/src/screenlog.cpp
	${CMAKE_SOURCE_DIR}/src/liveglsl.cpp
	${CMAKE_SOURCE_DIR}/src/renderpass.cpp
	${CMAKE_SOURCE_DIR}/src/shaderparser.cpp
	${CMAKE_SOURCE_DIR}/src/utils.cpp
	${CMAKE_SOURCE_DIR}/src/arguments.cpp
	${CMAKE_SOURCE_DIR}/deps/glfw/deps/glad.c
	${CMAKE_SOURCE_DIR}/deps/getopt/src/getopt.c)

add_executable(${EXECUTABLE_NAME} ${CMAKE_SOURCE_DIR}/src/main.cpp ${SOURCES})

target_link_libraries(${EXECUTABLE_NAME} glfw ${GLFW_LIBRARIES})

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES CMAKE_WIN32_EXECUTABLE ON)
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIRECTORY})
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY})
endif()

if (NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_executable(${EXECUTABLE_NAME}-tests ${CMAKE_SOURCE_DIR}/tests/liveglsl_tests.cpp ${SOURCES})

	target_link_libraries(${EXECUTABLE_NAME}-tests glfw ${GLFW_LIBRARIES})
endif()
