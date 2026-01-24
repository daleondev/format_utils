cmake_minimum_required(VERSION 3.20)

add_executable(format_sample)

target_sources(
    format_sample 
    PRIVATE 
    format_sample.cpp
)

target_link_libraries(
    format_sample
    PRIVATE
    format_utils::format_utils
    stdc++exp
)

target_compile_features(
    format_sample 
    PRIVATE 
    cxx_std_23
)