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

option(FMTU_ENABLE_JSON "Enable JSON support." ON)
option(FMTU_ENABLE_YAML "Enable YAML support." ON)
option(FMTU_ENABLE_TOML "Enable TOML support." ON)
if(FMTU_ENABLE_JSON OR FMTU_ENABLE_YAML OR FMTU_ENABLE_TOML)
    FetchContent_Declare(
        glaze
        GIT_REPOSITORY https://github.com/stephenberry/glaze.git
        GIT_TAG main
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(glaze)

    target_link_libraries(
        format_utils
        INTERFACE
        glaze::glaze
    )

    add_library(glaze_defines INTERFACE)
    target_compile_definitions(glaze_defines INTERFACE FMTU_ENABLE_GLAZE)

    if(FMTU_ENABLE_JSON)
        target_compile_definitions(glaze_defines INTERFACE FMTU_ENABLE_JSON)
    endif()
    if(FMTU_ENABLE_YAML)
        target_compile_definitions(glaze_defines INTERFACE FMTU_ENABLE_YAML)
    endif()
    if(FMTU_ENABLE_TOML)
        target_compile_definitions(glaze_defines INTERFACE FMTU_ENABLE_TOML)
    endif()

    target_link_libraries(
        format_utils
        INTERFACE
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