include(FetchContent)

FetchContent_Declare(
    reflect
    GIT_REPOSITORY  https://github.com/qlibs/reflect.git
    GIT_TAG         v1.2.6
    GIT_SHALLOW     TRUE
)
FetchContent_MakeAvailable(reflect)

add_library(reflect INTERFACE)
add_library(reflect::reflect ALIAS reflect)

target_include_directories(
    reflect 
    INTERFACE 
    ${reflect_SOURCE_DIR}
)

if(FMTU_ENABLE_JSON OR FMTU_ENABLE_YAML OR FMTU_ENABLE_TOML)
    FetchContent_Declare(
        glaze
        GIT_REPOSITORY  https://github.com/stephenberry/glaze.git
        GIT_TAG         main
        GIT_SHALLOW     TRUE
    )
    FetchContent_MakeAvailable(glaze)

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
endif()