
cmake_minimum_required(VERSION 2.8.12)

####################################################
#               Project Properties
####################################################

project("GLCore"
    DESCRIPTION "A simple interface to OpenGL"
    LANGUAGES CXX
)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

####################################################
#          Include Source File Directories
####################################################

set(DIRS "deps/tinyobjloader" "deps/imgui_modules")

####################################################
#           Create List of Source Files
####################################################

## Add Source Files from the main directory
file(GLOB SRC_FILES *.c *.cpp *.h *.hpp)
## Add Source Files from the other directories
foreach(DIR ${DIRS})
    # Find all source files & append to list
    file(GLOB SRC_FILES_IN_FOLDER ${DIR}/*.c ${DIR}/*.cpp ${DIR}/*.h ${DIR}/*.hpp)
    list(APPEND SRC_FILES ${SRC_FILES_IN_FOLDER})
endforeach()

## Show the files being added & put each file on it's own line
if(0)
    message("Compling Project '" ${PROJECT_NAME} "'\nSource Files:")
    foreach(SRC_FILE ${SRC_FILES}) 
        message(${SRC_FILE}) 
    endforeach()
endif()
####################################################
#                 Build Target
####################################################

## Build target with source files
add_library(${PROJECT_NAME} ${SRC_FILES}) # add_library / add_executable


####################################################
#                Include Librarys
####################################################


## Find dependencies

# WiringPi
#find_library(WIRING_PI_LIBRARIES NAMES wiringPi)
# pThreads
#find_package(Threads)
# Geos
#find_library(GEOS_LIBRARIES NAMES geos_c)

# GLFW (This was 3.3 ...)
find_package(glfw3 3.2.1 REQUIRED)
# OpenGL
find_package(OpenGL REQUIRED)
# Glew
find_package(GLEW REQUIRED)


## Include other cmakefiles
add_subdirectory(deps/imgui)

#find_package(glm REQUIRED)
#find_package(imgui REQUIRED)


# stb does not have a CMake build, but is header-only
#find_path(STB_INCLUDE_DIRS "stb.h") 


#target_include_directories(main PRIVATE ${STB_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    MaxLib
    ${WIRING_PI_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${GEOS_LIBRARIES}
    glfw
    OpenGL::GL
    GLEW::GLEW
    #glm::glm
    ImGui
)
####################################################
#             Set Compiler Options
####################################################

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic) 

####################################################
