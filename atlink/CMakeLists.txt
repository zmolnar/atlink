cmake_minimum_required(VERSION 3.20)
project(atlink VERSION 0.1.0 LANGUAGES C CXX)

# User-tweakable API namespace (propagated to consumers)
set(ATLINK_NAMESPACE "atlink" CACHE STRING "Atlink top-level enclosing namespace")

include(FetchContent)

FetchContent_Declare(
    magic_enum
    GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
    GIT_TAG        v0.9.7
    FIND_PACKAGE_ARGS NAMES magic_enum
)

FetchContent_Declare(
    GSL
    GIT_REPOSITORY https://github.com/microsoft/GSL.git
    GIT_TAG        v4.0.0
    FIND_PACKAGE_ARGS NAMES Microsoft.GSL
)

FetchContent_MakeAvailable(magic_enum gsl)

# Header-only library target
add_library(atlink INTERFACE)
add_library(atlink::atlink ALIAS atlink)

# Public headers (build tree and install tree)
target_include_directories(atlink
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
        $<INSTALL_INTERFACE:include>
)

# Public compile definitions (seen by dependents)
target_compile_definitions(atlink
    INTERFACE 
        ATL_NS=${ATLINK_NAMESPACE}
        MAGIC_ENUM_RANGE_MIN=0
        MAGIC_ENUM_RANGE_MAX=1024
)

target_link_libraries(atlink
    INTERFACE 
        magic_enum::magic_enum
        Microsoft.GSL::GSL
)

# Require C++17 for consumers
target_compile_features(atlink INTERFACE cxx_std_17)

# ---------- Install & Package ----------
include(CMakePackageConfigHelpers)

# Install headers (no sources to install)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/atlink DESTINATION include)

# Install/export the INTERFACE target
install(TARGETS atlink magic_enum GSL
    EXPORT atlinkTargets
)

install(EXPORT atlinkTargets
    NAMESPACE atlink::
    DESTINATION lib/cmake/atlink
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/atlinkConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# atlink/cmake/atlinkConfig.cmake.in should contain:
#   @PACKAGE_INIT@
#   include("${CMAKE_CURRENT_LIST_DIR}/atlinkTargets.cmake")
configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/cmake/atlinkConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/atlinkConfig.cmake"
    INSTALL_DESTINATION lib/cmake/atlink
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/atlinkConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/atlinkConfigVersion.cmake"
    DESTINATION lib/cmake/atlink
)

# Export a build-tree package too (so tests/examples can point at this dir)
export(EXPORT atlinkTargets
       FILE "${CMAKE_CURRENT_BINARY_DIR}/atlinkTargets.cmake"
       NAMESPACE atlink::)