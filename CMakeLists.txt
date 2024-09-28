cmake_minimum_required(VERSION 3.29)
project(PortManager C)

set(CMAKE_C_STANDARD 11)

add_executable(PortManager main.c
        ResponseHandler.h
)
