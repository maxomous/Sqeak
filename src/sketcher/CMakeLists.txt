
cmake_minimum_required(VERSION 2.8.12)

####################################################
#               Project Properties
####################################################

project("Sketcher"
    DESCRIPTION "A 2D Sketch Library integrated with ImGui"
    LANGUAGES CXX
)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

####################################################
#                   Include
####################################################

## Include other cmakefile projects
add_subdirectory(
    deps/constraintsolver
)

####################################################
#    Create List of Source Files in Directories
####################################################

set(DIRS "" "deps/constraintsolver")
## Add Source Files from the main directory
file(GLOB SRC_FILES *.c *.cpp *.h *.hpp)
## Add Source Files from the other directories
foreach(DIR ${DIRS})
    # Find all source files & append to list
    file(GLOB SRC_FILES_IN_FOLDER ${DIR}/*.c ${DIR}/*.cpp ${DIR}/*.h ${DIR}/*.hpp)
    list(APPEND SRC_FILES ${SRC_FILES_IN_FOLDER})
endforeach()

## Print the Project name and each of the Source files to be included
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
add_library(${PROJECT_NAME} ${SRC_FILES}) # add_library / add_executable


####################################################
#                Include Librarys
####################################################

target_link_libraries(${PROJECT_NAME}
    MaxLib
    ConstraintSolver
)

#add_definitions(-lMaxLib)

####################################################
#             Set Compiler Options
####################################################

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -g -O0 -Wno-psabi) #-O0 -v -Q for valgrind (try -Q for showing function names)

####################################################
