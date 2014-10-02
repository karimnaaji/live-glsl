cmake_minimum_required(VERSION 2.8)
project(ogl)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -stdlib=libc++ -std=c++0x")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/include")
set(CXX_FLAGS_DEBUG "-g -O0")

option(BUILD_TEST "Build test" ON)

file(GLOB_RECURSE OGL_SOURCES src/*.cpp)
file(GLOB_RECURSE OGL_HEADERS include/*.h)

set(OGL_INCLUDE_DIRS "")
foreach(_headerFile ${OGL_HEADERS})
	get_filename_component(_dir ${_headerFile} PATH)
	list(APPEND OGL_INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES OGL_INCLUDE_DIRS)
include_directories(${OGL_INCLUDE_DIRS})

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

add_subdirectory(${PROJECT_SOURCE_DIR}/deps/math/)
include_directories(${PROJECT_SOURCE_DIR}/deps/math/include)

include_directories(${OpenGL_INCLUDE_DIRS})
include_directories(${GLEW_INCLUDE_DIRS})

add_library(ogl STATIC ${OGL_SOURCES})
set(LIBS math ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})
target_link_libraries(ogl ${LIBS})

if(BUILD_TEST EQUAL ON) 
	message(STATUS "Build test on ogl library")
	add_executable(ogl.out ${OGL_SOURCES})
	target_link_libraries(ogl.out ${LIBS})
endif()
