cmake_minimum_required(VERSION 3.24)
project(
    format_utils
    VERSION 0.1.0
    DESCRIPTION "Formatting utilities"
    LANGUAGES CXX
)

option(FMTU_ENABLE_JSON "Enable JSON support." OFF)
option(FMTU_ENABLE_YAML "Enable YAML support." OFF)
option(FMTU_ENABLE_TOML "Enable TOML support." OFF)

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

add_library(format_utils_compiler_warnings INTERFACE)
add_library(format_utils::compiler_warnings ALIAS format_utils_compiler_warnings)
target_compile_options(
    format_utils_compiler_warnings
    INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

option(BUILD_SAMPLES "Build the sample executables." ON)
if(BUILD_SAMPLES)
    add_subdirectory(samples)
endif()

option(BUILD_TESTS "Build the tests." ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()