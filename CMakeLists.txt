cmake_minimum_required(VERSION 3.20)
project(
    format_utils
    VERSION 0.1.0
    DESCRIPTION "Formatting utilities"
    LANGUAGES CXX
)

include(FetchContent)

FetchContent_Declare(
    reflect
    GIT_REPOSITORY https://github.com/qlibs/reflect.git
    GIT_TAG        v1.2.6
)
FetchContent_MakeAvailable(reflect)
add_library(reflect INTERFACE)
add_library(reflect::reflect ALIAS reflect)
target_include_directories(
    reflect 
    INTERFACE 
    ${reflect_SOURCE_DIR}
)

add_library(format_utils INTERFACE)
add_library(format_utils::format_utils ALIAS format_utils)

target_sources(
    format_utils
    INTERFACE
    format_utils.hpp
)

target_include_directories(
    format_utils
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
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

option(BUILD_SAMPLES "Build the sample executables." ON)
if(BUILD_SAMPLES)
    add_subdirectory(samples)
endif()

option(BUILD_TESTS "Build the tests." ON)
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()