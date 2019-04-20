cmake_minimum_required (VERSION 2.6)
project (Crumble)
# The version number.
set (Crumble_VERSION_MAJOR 1)
set (Crumble_VERSION_MINOR 0) 

set(GLFW_INCLUDE_DIR "" CACHE FILEPATH "path to GLFW include")
set(GLFW_BINARY_DIR "" CACHE FILEPATH "path to GLFW binaries")

set(BULLET_INCLUDE_DIR "" CACHE FILEPATH "path to BULLET include")
set(BULLET_BINARY_DIR "" CACHE FILEPATH "path to BULLET binaries")
 
# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
include_directories("${PROJECT_BINARY_DIR}")
 
#Library include directories
include_directories("${GLFW_INCLUDE_DIR}")
include_directories("${BULLET_INCLUDE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/glad/include") 
include_directories("${PROJECT_SOURCE_DIR}/headerlibs")

#Add GLFW library 
add_library(GLFW STATIC IMPORTED)
set_target_properties(GLFW PROPERTIES IMPORTED_LOCATION ${GLFW_BINARY_DIR}/glfw3dll.lib)

#Add glad library
add_library(GLAD "${PROJECT_SOURCE_DIR}/glad/src/glad.c")

#Add bullet3 library
file(GLOB bulletSrc
    "${BULLET_BINARY_DIR}/*.lib"
)
#add_library(BULLET STATIC IMPORTED)

file(GLOB srcFiles
    "${PROJECT_SOURCE_DIR}/src/*.h"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_executable(Crumble "${PROJECT_SOURCE_DIR}/src/crumble.cpp" "${srcFiles}")

TARGET_LINK_LIBRARIES(Crumble GLFW GLAD opengl32 "${bulletSrc}")