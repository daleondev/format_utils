include(FetchContent)
FetchContent_Declare(
    GTest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.17.0
)
FetchContent_MakeAvailable(GTest)

add_executable(
    format_tests
    format_tests.cpp
    format_test_helpers.hpp
)

target_link_libraries(
    format_tests
    PRIVATE
    GTest::gtest_main
    GTest::gmock
    format_utils::format_utils
)

target_compile_features(
    format_tests
    INTERFACE
    cxx_std_23
)

enable_testing()
include(GoogleTest)
gtest_discover_tests(format_tests)