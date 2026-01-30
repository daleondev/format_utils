# FormatUtils

**FormatUtils** is a modern C++23 header-only library that extends `std::format` with powerful capabilities for automatic reflection, serialization, and formatted output of user-defined types. It seamlessly integrates with `std::format`, making it effortless to print complex data structures, including aggregates, classes with private members, enums, smart pointers, and optional values.

Key features include:
*   **Automatic Reflection:** Format structs and aggregates without writing any boilerplate code.
*   **Non-Intrusive Adapters:** specialized adapters to format classes with private members or custom layouts.
*   **Scoped Enum Support:** Automatically print enum names instead of integer values.
*   **Smart Pointer & Optional Support:** Built-in formatting for `std::unique_ptr`, `std::shared_ptr`, and `std::optional`.
*   **Serialization Integration:** Out-of-the-box support for **JSON**, **YAML**, and **TOML** via [Glaze](https://github.com/stephenberry/glaze).
*   **Format Specifiers:** Custom specifiers for verbose, pretty-print, and serialized output (e.g., `{:p}`, `{:j}`, `{:v}`).

## Requirements

*   **C++23** compatible compiler (GCC 13+, Clang 16+, MSVC 19.36+).
*   **CMake 3.24+**

## Installation

Since FormatUtils is header-only, you can simply include the `format_utils.hpp` file in your project.

### CMake FetchContent

You can easily include it in your CMake project:

```cmake
include(FetchContent)

FetchContent_Declare(
    format_utils
    GIT_REPOSITORY https://github.com/your-repo/format_utils.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(format_utils)

target_link_libraries(your_target PRIVATE format_utils::format_utils)
```

## Usage Examples

Include the header:
```cpp
#include "format_utils.hpp"
#include <print>
```

### 1. Automatic Reflection (Aggregates)

FormatUtils automatically reflects aggregate structures using C++23 reflection capabilities.

```cpp
struct Point {
    int x;
    int y;
};

struct Config {
    int id;
    std::string name;
    std::vector<double> values;
    Point resolution;
    bool is_active;
};

int main() {
    Config cfg{ 101, "SimulationConfig", { 0.5, 1.2, 3.14 }, { 1920, 1080 }, true };
    
    // Default format (Compact)
    std::println("{}", cfg);
    // Output: [ Config: { id: 101, name: SimulationConfig, values: [0.5, 1.2, 3.14], resolution: [ Point: { x: 1920, y: 1080 } ], is_active: true } ]

    // Pretty format (Indented) - {:p}
    std::println("{:p}", cfg);
    /* Output:
    Config: {
      id: 101,
      name: SimulationConfig,
      values: [0.5, 1.2, 3.14],
      resolution: {
        x: 1920,
        y: 1080
      },
      is_active: true
    }
    */
}
```

### 2. Adapters (Encapsulated Classes)

For classes with private members, define a `fmtu::Adapter` specialization.

```cpp
class User {
public:
    User(std::string name, std::string role) : m_name(name), m_role(role) {}
    const std::string& getName() const { return m_name; }
    const std::string& getRole() const { return m_role; }

private:
    std::string m_name;
    std::string m_role;
};

// Register the adapter
template<>
struct fmtu::Adapter<User> {
    using Fields = std::tuple<
        fmtu::Field<"name", &User::getName>,
        fmtu::Field<"role", &User::getRole>
    >;
};

int main() {
    User user("Alice", "Admin");
    std::println("{}", user);
    // Output: [ User: { name: Alice, role: Admin } ]
}
```

### 3. Scoped Enums

Scoped enums are automatically formatted by their name.

```cpp
enum class Status { Idle, Processing, Completed };

int main() {
    Status s = Status::Processing;
    
    // Default
    std::println("{}", s);   // Output: Processing
    
    // Verbose - {:v}
    std::println("{:v}", s); // Output: Status::Processing
}
```

### 4. Pointers & Optionals

FormatUtils handles `nullptr`, `std::optional`, and smart pointers gracefully.

```cpp
std::optional<int> opt_val = 123;
std::println("{}", opt_val); // Output: [ 123 ]

std::optional<int> empty_opt;
std::println("{}", empty_opt); // Output: [ null ]

auto ptr = std::make_unique<Point>(10, 20);
std::println("{}", ptr); 
// Output: [ (0x...) -> [ Point: { x: 10, y: 20 } ] ]
```

### 5. Serialization (JSON / TOML / YAML)

If enabled (via CMake options `FMTU_ENABLE_JSON`, etc.), you can format objects directly into serialized strings using **Glaze**.

**Format Specifiers:**
*   `{:j}` - Compact JSON
*   `{:pj}` - Pretty JSON
*   `{:t}` - TOML
*   `{:y}` - YAML (Experimental)

```cpp
Config cfg{ ... };

// JSON
std::println("{:j}", cfg);
// Output: {"id":101,"name":"SimulationConfig",...}

// Pretty JSON
std::println("{:pj}", cfg);

// TOML
std::println("{:t}", cfg);
```

## Build Instructions

To build the library, samples, and tests:

```bash
cmake -S . -B build --preset debug
cmake --build build --preset debug
```

Run the sample:
```bash
./build/Debug/samples/format_sample
```

Run tests:
```bash
./build/Debug/tests/format_tests
```

## Configuration Options

| Option | Description | Default |
| :--- | :--- | :--- |
| `FMTU_ENABLE_JSON` | Enable JSON support via Glaze | `ON` |
| `FMTU_ENABLE_TOML` | Enable TOML support via Glaze | `ON` |
| `FMTU_ENABLE_YAML` | Enable YAML support via Glaze | `OFF` |
| `BUILD_SAMPLES` | Build sample executables | `ON` |
| `BUILD_TESTS` | Build unit tests | `ON` |
