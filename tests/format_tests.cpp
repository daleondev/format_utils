#include "format_utils.hpp"

#include <gtest/gtest.h>

#include <format>
#include <memory>
#include <optional>

static auto split_str(const std::string& ptr_str, char delimiter) -> std::vector<std::string>
{
    auto split_view = ptr_str | std::views::split(delimiter);
    return split_view |
           std::views::transform([](auto&& str) { return std::string(str.begin(), str.end()); }) |
           std::ranges::to<std::vector>();
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

struct SimpleAggregate
{
    int id;
    double value;
    bool active;
};

struct NestedAggregate
{
    std::string name;
    SimpleAggregate simple;
};

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

enum class TestEnum
{
    ValueA,
    ValueB,
    ValueC
};

TEST(FormatTests, Aggregate_Compact)
{
    SimpleAggregate agg{ 42, 3.14, true };
    // Expected format: [ SimpleAggregate: {{ id: 42, value: 3.14, active: true }} ]
    std::string result = std::format("{}", agg);

    EXPECT_TRUE(result.starts_with("[ SimpleAggregate: {{ "));
    EXPECT_TRUE(result.ends_with(" }} ]"));
    EXPECT_NE(result.find("id: 42"), std::string::npos);
    EXPECT_NE(result.find("value: 3.14"), std::string::npos);
    EXPECT_NE(result.find("active: true"), std::string::npos);
}

TEST(FormatTests, Aggregate_Pretty)
{
    SimpleAggregate agg{ 42, 3.14, true };
    // Pretty format uses multi-line indentation
    std::string result = std::format("{:p}", agg);

    EXPECT_TRUE(result.starts_with("SimpleAggregate: {{\n"));
    EXPECT_NE(result.find("  id: 42"), std::string::npos);
    EXPECT_NE(result.find("  value: 3.14"), std::string::npos);
    EXPECT_NE(result.find("  active: true"), std::string::npos);
    EXPECT_TRUE(result.ends_with("}}"));
}

TEST(FormatTests, Aggregate_Nested)
{
    NestedAggregate nested{ "Parent", { 1, 1.0, false } };
    std::string result = std::format("{}", nested);

    // Verify nested structure inclusion
    EXPECT_NE(result.find("name: Parent"), std::string::npos);
    EXPECT_NE(result.find("simple: [ SimpleAggregate: {{"), std::string::npos);
}

// -----------------------------------------------------------------------------
// Test Suite: Adapters (Encapsulated Classes)
// -----------------------------------------------------------------------------

TEST(FormatTests, Adapter_Compact)
{
    ClassWithAdapter obj(100, "TestObj");
    std::string result = std::format("{}", obj);

    // Expected: [ ClassWithAdapter: {{ id: 100, name: TestObj }} ]
    EXPECT_TRUE(result.starts_with("[ ClassWithAdapter: {{ "));
    EXPECT_NE(result.find("id: 100"), std::string::npos);
    EXPECT_NE(result.find("name: TestObj"), std::string::npos);
    EXPECT_TRUE(result.ends_with(" }} ]"));
}

TEST(FormatTests, Adapter_Pretty)
{
    ClassWithAdapter obj(100, "TestObj");
    std::string result = std::format("{:p}", obj);

    EXPECT_TRUE(result.starts_with("ClassWithAdapter: {{\n"));
    EXPECT_NE(result.find("  id: 100"), std::string::npos);
    EXPECT_NE(result.find("  name: TestObj"), std::string::npos);
    EXPECT_TRUE(result.ends_with("}}"));
}

// -----------------------------------------------------------------------------
// Test Suite: Enums
// -----------------------------------------------------------------------------

TEST(FormatTests, Enum_Default)
{
    TestEnum e = TestEnum::ValueB;
    // Default: just the name "ValueB"
    EXPECT_EQ(std::format("{}", e), "ValueB");
}

TEST(FormatTests, Enum_Verbose)
{
    TestEnum e = TestEnum::ValueC;
    // Verbose: "TestEnum::ValueC"
    EXPECT_EQ(std::format("{:v}", e), "TestEnum::ValueC");
}

// -----------------------------------------------------------------------------
// Test Suite: Optionals
// -----------------------------------------------------------------------------

TEST(FormatTests, Optional_HasValue)
{
    std::optional<int> opt = 123;
    std::string result = std::format("{}", opt);
    // Expected: [ 123 ]
    EXPECT_EQ(result, "[ 123 ]");
}

TEST(FormatTests, Optional_Empty)
{
    std::optional<int> opt = std::nullopt;
    std::string result = std::format("{}", opt);
    // Expected: [ null ]
    EXPECT_EQ(result, "[ null ]");
}

TEST(FormatTests, Optional_Complex)
{
    std::optional<SimpleAggregate> opt = SimpleAggregate{ 1, 2.0, false };
    std::string result = std::format("{}", opt);

    EXPECT_TRUE(result.starts_with("[ [ SimpleAggregate: {{"));
    EXPECT_TRUE(result.ends_with("}} ] ]"));
}

// -----------------------------------------------------------------------------
// Test Suite: Pointers
// -----------------------------------------------------------------------------

TEST(FormatTests, Pointer_Raw)
{
    int value = 99;
    int* ptr = &value;
    std::string result = std::format("{}", ptr);

    auto parsed = parse_ptr_str(result);
    ASSERT_TRUE(parsed.has_value());
    EXPECT_TRUE(parsed->ptr.starts_with('(')); // Address in hex
    EXPECT_EQ(parsed->data, "99");
}

TEST(FormatTests, Pointer_Smart)
{
    auto ptr = std::make_unique<int>(55);
    std::string result = std::format("{}", ptr);

    // Smart pointer formatting delegates to the raw pointer formatter logic
    auto parsed = parse_ptr_str(result);
    ASSERT_TRUE(parsed.has_value());
    EXPECT_EQ(parsed->data, "55");
}

TEST(FormatTests, Pointer_Null)
{
    int* ptr = nullptr;
    // Implementation defines null pointer format, usually something like "0x0" or similar for void*,
    // but our formatter specializes ValuePtr to show "[ (0) -> null ]" pattern if nullptr.

    std::string result = std::format("{}", ptr);
    EXPECT_NE(result.find("null"), std::string::npos);
}

// -----------------------------------------------------------------------------
// Test Suite: Serialization (JSON / TOML)
// -----------------------------------------------------------------------------

#ifdef FMTU_ENABLE_JSON
TEST(FormatTests, JSON_Compact)
{
    SimpleAggregate agg{ 10, 20.5, true };
    // {:j} -> Compact JSON
    std::string result = std::format("{:j}", agg);

    // Simple checks for JSON structure
    EXPECT_NE(result.find("\"id\":10"), std::string::npos);
    EXPECT_NE(result.find("\"value\":20.5"), std::string::npos);
    EXPECT_NE(result.find("\"active\":true"), std::string::npos);
}

TEST(FormatTests, JSON_Pretty)
{
    SimpleAggregate agg{ 10, 20.5, true };
    // {:pj} -> Pretty JSON
    std::string result = std::format("{:pj}", agg);

    // Check for newlines/indentation which implies prettified output
    EXPECT_NE(result.find("\n"), std::string::npos);
    EXPECT_NE(result.find("  \"id\": 10"), std::string::npos); // Glaze pretty print default indent
}
#endif

#ifdef FMTU_ENABLE_TOML
TEST(FormatTests, TOML_Basic)
{
    SimpleAggregate agg{ 10, 20.5, true };
    // {:t} -> TOML
    std::string result = std::format("{:t}", agg);

    EXPECT_NE(result.find("id = 10"), std::string::npos);
    EXPECT_NE(result.find("value = 20.5"), std::string::npos);
    EXPECT_NE(result.find("active = true"), std::string::npos);
}
#endif

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}