cmake_minimum_required(VERSION 3.10)
project(live-glsl)

option(MACOS_ASAN "Enable ASAN on macOS" OFF)

set(EXECUTABLE_NAME live-glsl)
set(CMAKE_CXX_STANDARD 17)

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(CMAKE_WIN32_EXECUTABLE ON)
else()
    if (MACOS_ASAN)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wno-unused-result -Wno-deprecated-declarations")
    endif()
endif()

include_directories(${CMAKE_SOURCE_DIR}/deps)
include_directories(${CMAKE_SOURCE_DIR}/deps/glfw/include)
include_directories(${CMAKE_SOURCE_DIR}/deps/glad/include)
include_directories(${CMAKE_SOURCE_DIR}/deps/getopt/include)
include_directories(${CMAKE_SOURCE_DIR}/tests)
include_directories(${CMAKE_SOURCE_DIR}/src)

if (NOT EMSCRIPTEN)
    option(GLFW_BUILD_EXAMPLES OFF)
    option(GLFW_BUILD_TESTS OFF)
    option(GLFW_BUILD_DOCS OFF)
    option(GLFW_INSTALL OFF)

    add_subdirectory(${CMAKE_SOURCE_DIR}/deps/glfw)
else()
    set(LFLAGS "${LFLAGS} -s USE_GLFW=3")
    set(LFLAGS "${LFLAGS} -s ASSERTIONS=1")
    set(LFLAGS "${LFLAGS} -s MAX_WEBGL_VERSION=2")
    set(LFLAGS "${LFLAGS} -s ALLOW_MEMORY_GROWTH=1")
endif()

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

if (EMSCRIPTEN)
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES LINK_FLAGS "${LFLAGS}")
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES SUFFIX ".html")
    set(SHADER_INPUT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/shaders/simple.frag")
    set(SHADER_INPUT_DST "simple.frag")
    set(EMSCRIPTEN_GENERATE_HTML ON)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file ${SHADER_INPUT_SRC}@${SHADER_INPUT_DST} --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/shell.html")
    target_compile_definitions(${EXECUTABLE_NAME} PRIVATE -DSHADER_INPUT="${SHADER_INPUT_DST}")
endif()

target_link_libraries(${EXECUTABLE_NAME} glfw)

if (NOT EMSCRIPTEN)
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
endif()
