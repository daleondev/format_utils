#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <ranges>
#include <algorithm>

static auto split_str(const std::string &ptr_str, char delimiter) -> std::vector<std::string>
{
    // clang-format off
    auto split_view = ptr_str | std::views::split(delimiter);
    return split_view | 
        std::views::transform([](auto &&str){ return std::string(str.begin(), str.end()); }) |
        std::ranges::to<std::vector>();
    // clang-format on
}

auto is_valid_hex(std::string_view s) -> bool
{
    if (s.empty())
        return false;

    if (!s.starts_with("0x") && !s.starts_with("0X"))
        return false;

    s.remove_prefix(2);

    return std::ranges::all_of(s, [](unsigned char c)
                               { return std::isxdigit(c); });
}

struct PtrData
{
    std::string ptr;
    std::string data;
};

static auto parse_ptr_str(const std::string &ptr_str) -> std::optional<PtrData>
{
    PtrData ptr_data{};
    auto parts{split_str(ptr_str, ' ')};
    if (parts[0] != "[")
        return std::nullopt;
    ptr_data.ptr = parts[1];
    if (parts[2] != "->")
        return std::nullopt;
    ptr_data.data = parts[3];
    if (parts[4] != "]")
        return std::nullopt;
    return ptr_data;
}