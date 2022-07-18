
cmake_minimum_required(VERSION 2.8.12)

####################################################
#               Project Properties
####################################################

project("ConstraintSolver"
    DESCRIPTION "A geometric constraints solver wrapper for SolveSpaceLib"
    LANGUAGES CXX
)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

####################################################
#            Include
####################################################

## Include other cmakefile projects
add_subdirectory(
    libslvs
)

####################################################
#                 Build Target
####################################################

## Build target with source files
add_library(${PROJECT_NAME} 
    solver.cpp
) # add_library / add_executable


####################################################
#                Include Librarys
####################################################

target_link_libraries(${PROJECT_NAME}
    
    PUBLIC libslvs
)

####################################################
#             Set Compiler Options
####################################################

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -g -O0) #-O0 -v -Q for valgrind (try -Q for showing function names)

####################################################
