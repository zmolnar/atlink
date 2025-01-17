
cmake_minimum_required(VERSION 3.20)
project(atlink_sandbox LANGUAGES C CXX)

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Use atlink sources directly via add_subdirectory
add_subdirectory(../../atlink atlink_build)

add_executable(atlink_sandbox
    src/main.cpp
)

# Link against the atlink target
target_link_libraries(atlink_sandbox PRIVATE atlink::atlink)

set_target_properties(atlink_sandbox PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)