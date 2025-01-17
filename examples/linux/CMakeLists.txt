cmake_minimum_required(VERSION 3.10)

# Project name
project(ExampleProject)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Create an executable for your example program
add_executable(example_program main.cpp)

# Link the ATLink interface library
add_subdirectory(../../atlink ${CMAKE_BINARY_DIR}/atlink_build)

target_link_libraries(example_program PRIVATE ATLink)