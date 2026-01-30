#include "format_utils.hpp"

#include <gtest/gtest.h>

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// -----------------------------------------------------------------------------
// Test Suite: Aggregates (Automatic Reflection)
// -----------------------------------------------------------------------------

struct SimpleAggregate
{
    int id;
    double value;
    bool active;
};

TEST(FormatTests, Aggregate_Compact)
{
    std::string result = std::format("{}", SimpleAggregate{ 42, 3.14, true });
    std::string expected = "[ SimpleAggregate: { id: 42, value: 3.14, active: true } ]";
    EXPECT_EQ(result, expected);
}

TEST(FormatTests, Aggregate_Pretty)
{
    std::string result = std::format("{:p}", SimpleAggregate{ 42, 3.14, true });
    std::string expected = R"(SimpleAggregate: {
  id: 42,
  value: 3.14,
  active: true
})";
    EXPECT_EQ(result, expected);
}

struct NestedAggregate
{
    std::string name;
    SimpleAggregate simple;
};

TEST(FormatTests, Aggregate_Nested)
{
    std::string result = std::format("{}", NestedAggregate{ "Parent", { 1, 1.0, false } });
    std::string expected = "[ NestedAggregate: { name: Parent, simple: [ SimpleAggregate: { id: 1, value: 1, "
                           "active: false } ] } ]";
    EXPECT_EQ(result, expected);
}

#ifndef FMTU_ENABLE_GLAZE
struct AggregateWithNonFormattableMember
{
    int64_t id;
    std::mutex mutex;
};

TEST(FormatTests, Aggregate_NonFormattableMember)
{
    std::string result = std::format("{}", AggregateWithNonFormattableMember{ 12, {} });
    std::string expected = "[ AggregateWithNonFormattableMember: { id: 12, mutex: - } ]";
    EXPECT_EQ(result, expected);
}
#endif

// -----------------------------------------------------------------------------
// Test Suite: Adapters (Encapsulated Classes)
// -----------------------------------------------------------------------------

class ClassWithAdapter
{
  public:
    ClassWithAdapter(int id, std::string name)
      : m_id(id)
      , m_name(std::move(name))
    {
    }

    int getId() const { return m_id; }
    const std::string& getName() const { return m_name; }

  private:
    int m_id;
    std::string m_name;
};

template<>
struct fmtu::Adapter<ClassWithAdapter>
{
    using Fields = std::tuple<fmtu::Field<"id", &ClassWithAdapter::getId>,
                              fmtu::Field<"name", &ClassWithAdapter::getName>>;
};

TEST(FormatTests, Adapter_Compact)
{
    std::string result = std::format("{}", ClassWithAdapter{ 100, "TestObj" });
    std::string expected = "[ ClassWithAdapter: { id: 100, name: TestObj } ]";
    EXPECT_EQ(result, expected);
}

TEST(FormatTests, Adapter_Pretty)
{
    std::string result = std::format("{:p}", ClassWithAdapter{ 100, "TestObj" });
    std::string expected = R"(ClassWithAdapter: {
  id: 100,
  name: TestObj
})";
    EXPECT_EQ(result, expected);
}

// -----------------------------------------------------------------------------
// Test Suite: Enums
// -----------------------------------------------------------------------------

enum class TestEnum
{
    ValueA,
    ValueB,
    ValueC
};

TEST(FormatTests, Enum_Default)
{
    std::string result = std::format("{}", TestEnum::ValueB);
    std::string expected = "ValueB";
    EXPECT_EQ(result, expected);
}

TEST(FormatTests, Enum_Verbose)
{
    std::string result = std::format("{:v}", TestEnum::ValueC);
    std::string expected = "TestEnum::ValueC";
    EXPECT_EQ(result, expected);
}

// -----------------------------------------------------------------------------
// Test Suite: Optionals
// -----------------------------------------------------------------------------

TEST(FormatTests, Optional_HasValue)
{
    std::optional<int> opt = 123;
    std::string result = std::format("{}", opt);
    std::string expected = "[ 123 ]";
    EXPECT_EQ(result, expected);
}

TEST(FormatTests, Optional_Empty)
{
    std::optional<int> opt = std::nullopt;
    std::string result = std::format("{}", opt);
    std::string expected = "[ null ]";
    EXPECT_EQ(result, expected);
}

TEST(FormatTests, Optional_Complex)
{
    std::optional<SimpleAggregate> opt = SimpleAggregate{ 1, 2.2, false };
    std::string result = std::format("{}", opt);
    std::string expected = "[ [ SimpleAggregate: { id: 1, value: 2.2, active: false } ] ]";
    EXPECT_EQ(result, expected);
}
// -----------------------------------------------------------------------------
// Test Suite: Pointers
// -----------------------------------------------------------------------------

static auto split_str(const std::string& ptr_str, char delimiter) -> std::vector<std::string>
{
    auto split_view = ptr_str | std::views::split(delimiter);
    return split_view |
           std::views::transform([](auto&& str) { return std::string(str.begin(), str.end()); }) |
           std::ranges::to<std::vector>();
}

static auto is_valid_hex(std::string_view s) -> bool
{
    if (s.empty())
        return false;

    if (!s.starts_with("0x") && !s.starts_with("0X"))
        return false;

    s.remove_prefix(2);

    return std::ranges::all_of(s, [](unsigned char c) { return std::isxdigit(c); });
}

struct PtrData
{
    std::string ptr;
    std::string data;
};

static auto parse_ptr_str(const std::string& ptr_str) -> std::optional<PtrData>
{
    PtrData ptr_data{};
    auto parts{ split_str(ptr_str, ' ') };
    if (parts.size() < 5) {
        return std::nullopt;
    }
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

TEST(FormatTests, Pointer_Raw)
{
    int value = 99;
    int* ptr = &value;
    std::string result = std::format("{}", ptr);

    auto parsed = parse_ptr_str(result);
    ASSERT_TRUE(parsed.has_value());
    EXPECT_TRUE(parsed.value().ptr.starts_with('('));
    EXPECT_TRUE(parsed.value().ptr.ends_with(')'));
    EXPECT_TRUE(is_valid_hex(parsed.value().ptr.substr(1, parsed.value().ptr.size() - 2)));
    EXPECT_EQ(parsed->data, "99");
}

TEST(FormatTests, Pointer_Smart)
{
    auto ptr = std::make_unique<int>(55);
    std::string result = std::format("{}", ptr);

    auto parsed = parse_ptr_str(result);
    ASSERT_TRUE(parsed.has_value());
    EXPECT_TRUE(parsed.value().ptr.starts_with('('));
    EXPECT_TRUE(parsed.value().ptr.ends_with(')'));
    EXPECT_TRUE(is_valid_hex(parsed.value().ptr.substr(1, parsed.value().ptr.size() - 2)));
    EXPECT_EQ(parsed->data, "55");
}

TEST(FormatTests, Pointer_Null)
{
    int* ptr = nullptr;
    std::string result = std::format("{}", ptr);
    std::string expected = "[ (0x0) -> null ]";
    EXPECT_EQ(result, expected);
}

// -----------------------------------------------------------------------------
// Test Suite: Serialization (JSON / TOML)
// -----------------------------------------------------------------------------

#ifdef FMTU_ENABLE_JSON
TEST(FormatTests, JSON_Compact)
{
    std::string result = std::format("{:j}", SimpleAggregate{ 10, 20.5, true });
    std::string expected = R"({"id":10,"value":20.5,"active":true})";
    EXPECT_EQ(result, expected);
}

TEST(FormatTests, JSON_Pretty)
{
    std::string result = std::format("{:pj}", SimpleAggregate{ 10, 20.5, true });
    std::string expected = R"({
   "id": 10,
   "value": 20.5,
   "active": true
})";
    EXPECT_EQ(result, expected);
}
#endif

#ifdef FMTU_ENABLE_TOML
TEST(FormatTests, TOML_Basic)
{
    std::string result = std::format("{:t}", SimpleAggregate{ 10, 20.5, true });
    std::string expected = R"(id = 10
value = 20.5
active = true)";
    EXPECT_EQ(result, expected);
}
#endif
