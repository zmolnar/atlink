cmake_minimum_required(VERSION 3.10)

# Project name (not necessary but good practice)
project(ATLink)

# Create a header-only library (INTERFACE type)
add_library(ATLink INTERFACE)

# Define the include directory for ATLink headers
target_include_directories(ATLink
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}  # Location of ATLink headers
)