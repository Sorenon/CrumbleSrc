cmake_minimum_required (VERSION 2.6)
project (Crumble)
# The version number.
set (Crumble_VERSION_MAJOR 1)
set (Crumble_VERSION_MINOR 0) 

set(GLFW_INCLUDE_DIR "" CACHE FILEPATH "path to GLFW headers")
set(GLFW_BINARY_DIR "" CACHE FILEPATH "path to GLFW binaries")

set(BULLET_INCLUDE_DIR "" CACHE FILEPATH "path to BULLET headers")
set(BULLET_BINARY_DIR "" CACHE FILEPATH "path to BULLET binaries")

set(GNS_INCLUDE_DIR "" CACHE FILEPATH "path to GameNetworkingSockets headers")
set(GNS_BINARY_DIR "" CACHE FILEPATH "path to GameNetworkingSockets binaries")

set(BOOST_INCLUDE_DIR "" CACHE FILEPATH "path to the BOOST headers")
set(BOOST_BINARY_DIR "" CACHE FILEPATH "path to the BOOST binaries")
 
# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
include_directories("${PROJECT_BINARY_DIR}")

# prevent the need for UNIX directory shortcuts in including project headers
include_directories("${PROJECT_SOURCE_DIR}/src")
 
#Library include directories
include_directories("${GLFW_INCLUDE_DIR}")
include_directories("${BULLET_INCLUDE_DIR}")
include_directories("${GNS_INCLUDE_DIR}")
include_directories("${BOOST_INCLUDE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/glad/include")
include_directories("${PROJECT_SOURCE_DIR}/headerlibs")

#Add GLFW library 
add_library(GLFW STATIC IMPORTED)
set_target_properties(GLFW PROPERTIES IMPORTED_LOCATION "${GLFW_BINARY_DIR}/glfw3.lib")

#Add GameNetworkingSockets library
add_library(GNS STATIC IMPORTED)
set_target_properties(GNS PROPERTIES IMPORTED_LOCATION "${GNS_BINARY_DIR}/GameNetworkingSockets.lib")

#Add glad library
add_library(GLAD "${PROJECT_SOURCE_DIR}/glad/src/glad.c")

#Add bullet3 library
file(GLOB bulletSrc
    "${BULLET_BINARY_DIR}/*.lib"
)

#Add boost library
file(GLOB boostSrc
    "${BOOST_BINARY_DIR}/*.lib"
)

file(GLOB_RECURSE sourceFiles
    "${PROJECT_SOURCE_DIR}/src/*"
)

foreach(FILE ${sourceFiles})
	get_filename_component(RELDIR ${FILE} DIRECTORY )
	file(RELATIVE_PATH PARENT_DIR "${PROJECT_SOURCE_DIR}/src" ${RELDIR})

    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")

    string(REPLACE "/" "\\" GROUP "${GROUP}")

    if ("${FILE}" MATCHES ".*\\.cpp")
       set(GROUP "Source Files\\${GROUP}")
    elseif("${FILE}" MATCHES ".*\\.h")
       set(GROUP "Header Files\\${GROUP}")
	else()
       set(GROUP "Resource Files\\${GROUP}")
    endif()

    source_group("${GROUP}" FILES "${FILE}")
endforeach()

add_executable(Crumble "${PROJECT_SOURCE_DIR}/src/crumble.cpp" ${sourceFiles})

set_target_properties(Crumble PROPERTIES CXX_STANDARD 17)

TARGET_LINK_LIBRARIES(Crumble GLFW GLAD GNS opengl32 "${bulletSrc}" "${boostSrc}")