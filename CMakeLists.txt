cmake_minimum_required(VERSION 3.24)
project(
    format_utils
    VERSION 0.1.0
    DESCRIPTION "Formatting utilities"
    LANGUAGES CXX
)

option(FMTU_ENABLE_JSON "Enable JSON support." ON)
option(FMTU_ENABLE_YAML "Enable YAML support." ON)
option(FMTU_ENABLE_TOML "Enable TOML support." ON)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/deps.cmake)

add_library(format_utils INTERFACE)
add_library(format_utils::format_utils ALIAS format_utils)

target_sources(
    format_utils
    INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_LIST_DIR}
    FILES format_utils.hpp
)

target_link_libraries(
    format_utils
    INTERFACE
    reflect::reflect
)

target_compile_features(
    format_utils
    INTERFACE
    cxx_std_23
)

if(FMTU_ENABLE_JSON OR FMTU_ENABLE_YAML OR FMTU_ENABLE_TOML)
    target_link_libraries(
        format_utils
        INTERFACE
        glaze::glaze
        glaze_defines
    )
endif()

option(BUILD_SAMPLES "Build the sample executables." ON)
if(BUILD_SAMPLES)
    add_subdirectory(samples)
endif()

option(BUILD_TESTS "Build the tests." ON)
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()