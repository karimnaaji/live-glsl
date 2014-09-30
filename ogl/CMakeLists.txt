file(GLOB_RECURSE OGL_SOURCES *.cpp)
file(GLOB_RECURSE OGL_HEADERS *.h)

set(OGL_INCLUDE_DIRS "")
foreach(_headerFile ${OGL_HEADERS})
	get_filename_component(_dir ${_headerFile} PATH)
	list(APPEND OGL_INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES OGL_INCLUDE_DIRS)
include_directories(${OGL_INCLUDE_DIRS})

add_library(ogl STATIC ${OGL_SOURCES})

find_package(OpenGL REQUIRED)
find_package(GLM REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW REQUIRED)

include_directories(${GLEW_INCLUDE_DIRS})
include_directories(${OpenGL_INCLUDE_DIRS})
include_directories(${GLM_INCLUDE_DIRS})
include_directories(${GLFW_INCLUDE_DIR})

target_link_libraries(ogl ${GLFW_LIBRARY} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})