#include "format_utils.hpp"
#include "format_test_helpers.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

TEST(format_utils, format_ptr)
{
    int *value_ptr = new int(17);
    auto value_ptr_data{parse_ptr_str(std::format("{}", value_ptr))};
    ASSERT_TRUE(value_ptr_data.has_value());
    EXPECT_TRUE(value_ptr_data.value().ptr.starts_with('('));
    EXPECT_TRUE(value_ptr_data.value().ptr.ends_with(')'));
    EXPECT_TRUE(is_valid_hex(value_ptr_data.value().ptr.substr(1, value_ptr_data.value().ptr.size() - 2)));
    EXPECT_STREQ(value_ptr_data.value().data.c_str(), std::format("{}", *value_ptr).c_str());

    void *void_ptr{nullptr};
    EXPECT_STREQ(std::format("{}", void_ptr).c_str(), "0x0");
}