cmake_minimum_required(VERSION 3.5)
set( CMAKE_CXX_COMPILER "/usr/bin/g++" )

project(client LANGUAGES CXX)

set(CMAKE_VERBOSE_MAKEFILE ON)

file(GLOB SOURCES CONFIGURE_DEPENDS
./src/*.cpp
./headers/*.h
)

add_executable(client.out ${SOURCES})