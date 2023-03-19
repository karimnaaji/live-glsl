cmake_minimum_required(VERSION 3.10)
project(live-glsl)

option(MACOS_ASAN "Enable ASAN on macOS" OFF)

set(EXECUTABLE_NAME live-glsl)
set(CMAKE_CXX_STANDARD 11)

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(CMAKE_WIN32_EXECUTABLE ON)
else()
    if (MACOS_ASAN)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    else()
        #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wno-unused-result -Wno-deprecated-declarations")
    endif()
endif()

include_directories(${CMAKE_SOURCE_DIR}/deps)
include_directories(${CMAKE_SOURCE_DIR}/deps/glfw/include)
include_directories(${CMAKE_SOURCE_DIR}/deps/glad/include)
include_directories(${CMAKE_SOURCE_DIR}/deps/getopt/include)
include_directories(${CMAKE_SOURCE_DIR}/tests)
include_directories(${CMAKE_SOURCE_DIR}/src)

option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_BUILD_DOCS OFF)
option(GLFW_INSTALL OFF)

add_subdirectory(${CMAKE_SOURCE_DIR}/deps/glfw)

set(SOURCES
    ${CMAKE_SOURCE_DIR}/src/gui.cpp
    ${CMAKE_SOURCE_DIR}/src/filewatcher.cpp
    ${CMAKE_SOURCE_DIR}/src/shader.cpp
    ${CMAKE_SOURCE_DIR}/src/liveglsl.cpp
    ${CMAKE_SOURCE_DIR}/src/renderpass.cpp
    ${CMAKE_SOURCE_DIR}/src/shaderparser.cpp
    ${CMAKE_SOURCE_DIR}/src/utils.cpp
    ${CMAKE_SOURCE_DIR}/src/arguments.cpp
    ${CMAKE_SOURCE_DIR}/deps/glad/src/gl.c
    ${CMAKE_SOURCE_DIR}/deps/microui/microui.c
    ${CMAKE_SOURCE_DIR}/deps/getopt/src/getopt.c)

add_executable(${EXECUTABLE_NAME} ${CMAKE_SOURCE_DIR}/src/main.cpp ${SOURCES})

target_link_libraries(${EXECUTABLE_NAME} glfw)

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIRECTORY})
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY})
endif()

if (NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_executable(${EXECUTABLE_NAME}-tests ${CMAKE_SOURCE_DIR}/tests/liveglsl_tests.cpp ${SOURCES})

    target_link_libraries(${EXECUTABLE_NAME}-tests glfw ${GLFW_LIBRARIES})
endif()
