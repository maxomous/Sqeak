
cmake_minimum_required(VERSION 2.8.12)

####################################################
#               Project Properties
####################################################

project("Sqeak"
    DESCRIPTION "A GRBL GCode streamer, Graphical GCode Viewer + Basic CAD/CAM Package"
    LANGUAGES CXX
)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/..")

####################################################
#          Include Source File Directories
####################################################

set(DIRS "dev" "libs" "gui" "functions" "sketch" )
####################################################
#           Create List of Source Files
####################################################

### # Build list of header files to install. Root == "."
### set(HEADER_DIRS "." "File" "Geom")
### ## Add Source Files from the other directories
### foreach(DIR ${HEADER_DIRS})
###     # Find all source files & append to list
###     if(DIR STREQUAL ".")
###         file(GLOB HEADER_FILES_IN_FOLDER *.h)
###     else()
###         file(GLOB HEADER_FILES_IN_FOLDER ${DIR}/*.h)
###     endif()
###     list(APPEND HEADER_FILES ${HEADER_FILES_IN_FOLDER})
### endforeach()


## Add Source Files from the main directory
file(GLOB SRC_FILES *.c *.cpp *.h *.hpp)
## Add Source Files from the other directories
foreach(DIR ${DIRS})
    # Find all source files & append to list
    file(GLOB SRC_FILES_IN_FOLDER ${DIR}/*.c ${DIR}/*.cpp ${DIR}/*.h ${DIR}/*.hpp)
    list(APPEND SRC_FILES ${SRC_FILES_IN_FOLDER})
endforeach()

## Show the files being added & put each file on it's own line
if(1)
    message("Compling Project '" ${PROJECT_NAME} "'\nSource Files:")
    foreach(SRC_FILE ${SRC_FILES}) 
        message(${SRC_FILE}) 
    endforeach()
endif()

####################################################
#                 Build Target
####################################################

## Build target with source files
add_executable(${PROJECT_NAME} ${SRC_FILES}) # add_library / add_executable


####################################################
#                Include Librarys
####################################################

## Include Library from cmakefile
add_subdirectory(glcore)
add_subdirectory(sketcher)
add_subdirectory(grbl)

## Include Library from source
#target_include_directories()
#add_library() or similar in other cmakefile


# Geos

# link libraries
target_link_libraries(${PROJECT_NAME}
    MaxLib
    geos_c
    geos 
    GLCore
    Sketcher
    GRBL
)



#-lGL -lglfw -lGLEW 
#add_definitions(-lgeos_c)

# Current working directory
#set(THIS_DIR ${CMAKE_CURRENT_SOURCE_DIR})

### Linking a library ###
#include_directories( stb_image )
#link_directories(stb_image)

#find_library(MYLIB stb_image)
#target_link_libraries(${PROJECT_NAME} PUBLIC ${MYLIB})


####################################################
#             Set Compiler Options
####################################################

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -g -O0 -Wno-psabi) 
#-O0 -v -Q for valgrind (try -Q for showing function names)
# -Wno-psabi hides "changed in GCC 7.1" warning

####################################################
