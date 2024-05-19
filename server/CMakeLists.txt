cmake_minimum_required(VERSION 3.5)
set( CMAKE_CXX_COMPILER "/usr/bin/g++" )

project(server LANGUAGES CXX)

file(GLOB SOURCES
    headers/*.h
    headers/*.hpp
    src/*.cpp
)

add_executable(server.out ${SOURCES})