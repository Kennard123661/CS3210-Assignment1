cmake_minimum_required(VERSION 3.12)
project(cs3210_assignment1 C)

set(CMAKE_C_STANDARD 11)

add_executable(cs3210_assignment1 main.c LineNetwork.c LineNetwork.h StationLink.c StationLink.h Train.c Train.h)