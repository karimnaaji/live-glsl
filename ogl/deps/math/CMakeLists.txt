cmake_minimum_required(VERSION 2.8)
project(math)

file(GLOB_RECURSE MATH_SOURCES src/*.cpp)
file(GLOB_RECURSE MATH_HEADERS include/*.h)

set(MATH_INCLUDE_DIRS "")
foreach(_headerFile ${MATH_HEADERS})
	get_filename_component(_dir ${_headerFile} PATH)
	list(APPEND MATH_INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES MATH_INCLUDE_DIRS)
include_directories(${MATH_INCLUDE_DIRS})

add_library(math STATIC ${MATH_SOURCES})