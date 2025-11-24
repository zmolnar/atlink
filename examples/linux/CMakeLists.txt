
cmake_minimum_required(VERSION 3.20)
project(atlink_example_linux LANGUAGES C CXX)

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Use atlink sources directly via add_subdirectory
add_subdirectory(../../atlink atlink_build)

add_executable(atlink_example_linux
    src/main.cpp
)

target_include_directories(atlink_example_linux PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(atlink_example_linux PRIVATE
  AT_PLATFORM_LINUX
)

target_compile_options(atlink_example_linux PRIVATE
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wno-deprecated-declarations>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4996>
)

# Link against the atlink target
target_link_libraries(atlink_example_linux PRIVATE atlink::atlink)

set_target_properties(atlink_example_linux PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)