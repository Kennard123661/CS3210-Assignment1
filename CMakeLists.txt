cmake_minimum_required(VERSION 3.12)
project(cs3210_assignment1 C)

set(CMAKE_C_STANDARD 11)

add_executable(cs3210_assignment1 main.c LineNetwork.c LineNetwork.h Train.c Train.h StationWait.h StationWait.c)

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()
