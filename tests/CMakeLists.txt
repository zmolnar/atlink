cmake_minimum_required(VERSION 3.20)
project(atlink_tests LANGUAGES C CXX)

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Catch2 3 REQUIRED)

enable_testing()

# Use atlink sources directly via add_subdirectory
add_subdirectory(../atlink atlink_build)

add_executable(atlink_tests
    utDeserializer.cpp
    utEnumStringConverter.cpp
    utResponse.cpp
    utResponsePack.cpp
    utCommand.cpp
    utUrc.cpp
)

target_link_libraries(atlink_tests PRIVATE atlink::atlink Catch2::Catch2WithMain)

set_target_properties(atlink_tests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)