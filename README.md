# format_utils

[![Multi-Platform CI/CD](https://github.com/daleondev/format_utils/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/daleondev/format_utils/actions/workflows/cmake-multi-platform.yml)

**format_utils** is a modern C++23 header-only library that extends `std::format` with powerful capabilities for automatic reflection, serialization, and formatted output of user-defined types. It seamlessly integrates with `std::format`, making it effortless to print complex data structures, including aggregates, classes with private members, enums, pointers, and optional values.

Key features include:
*   **Automatic Reflection:** Format structs and aggregates without writing any boilerplate code.
*   **Non-Intrusive Adapters:** Specialized adapters to format classes with private members or custom layouts.
*   **Enum Support:** Automatically print scoped enum names instead of integer values.
*   **Pointer & Optional Support:** Built-in formatting for raw pointers, smart pointers, and `std::optional`.
*   **Serialization Integration:** Out-of-the-box support for **JSON**, **YAML**, and **TOML** via [Glaze](https://github.com/stephenberry/glaze).
*   **Format Specifiers:** Custom specifiers for verbose, pretty-print, and serialized output (e.g., `{:p}`, `{:j}`, `{:v}`).

## Requirements

*   **C++23** compatible compiler (GCC 14+, Clang 18+, MSVC 19.36+).
*   **CMake 3.24+**

## Usage Examples

Include the header:
```cpp
#include "format_utils.hpp"
#include <print>
```

### 1. Automatic Reflection (Aggregates)

format_utils automatically reflects aggregate structures.

```cpp
struct Point
{
    int x;
    int y;
};

struct Config
{
    int id;
    std::string name;
    std::vector<double> values;
    Point resolution;
    bool is_active;
};

int main()
{
    Config cfg{ 101, "SimulationConfig", { 0.5, 1.2, 3.14 }, { 1920, 1080 }, true };

    std::println("{}", cfg);
    // Output: [ Config: { id: 101, name: SimulationConfig, values: [0.5, 1.2, 3.14], 
    // resolution: [ Point: { x: 1920, y: 1080 } ], is_active: true } ]

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
class User
{
  public:
    User(std::string name, std::string role)
      : m_name(name)
      , m_role(role)
    {
    }
    const std::string& getName() const { return m_name; }
    const std::string& getRole() const { return m_role; }

  private:
    std::string m_name;
    std::string m_role;
};

template<>
struct fmtu::Adapter<User>
{
    using Fields = std::tuple<
        fmtu::Field<"name", &User::getName>, 
        fmtu::Field<"role", &User::getRole>
    >;
};

int main()
{
    User user("Alice", "Admin");

    std::println("{}", user);
    // Output: [ User: { name: Alice, role: Admin } ]
}
```

### 3. Scoped Enums

Scoped enums are automatically formatted by their name.

```cpp
enum class Status
{
    Idle,
    Processing,
    Completed
};

int main()
{
    Status s = Status::Processing;

    std::println("{}", s); 
    // Output: Processing

    std::println("{:v}", s); 
    // Output: Status::Processing
}
```

### 4. Pointers & Optionals

FormatUtils handles `nullptr`, `std::optional`, and smart pointers gracefully.

```cpp
#include <memory>
#include <optional>

struct Point
{
    int x;
    int y;
};

int main()
{
    std::optional<int> opt_val = 123;
    std::println("{}", opt_val); 
    // Output: [ 123 ]

    std::optional<int> empty_opt;
    std::println("{}", empty_opt); 
    // Output: [ null ]

    auto ptr = std::make_unique<Point>(10, 20);
    std::println("{}", ptr);
    // Output: [ (0x1d43c09c1f0) -> [ Point: { x: 10, y: 20 } ] ]
}
```

### 5. Serialization (JSON / TOML / YAML)

If enabled (via CMake options `FMTU_ENABLE_JSON`, etc.), you can format objects directly into serialized strings using **Glaze**.

**Format Specifiers:**
*   `{:j}` - Compact JSON
*   `{:pj}` - Pretty JSON
*   `{:y}` - YAML (experimental)
*   `{:t}` - TOML

```cpp
struct Point
{
    int x;
    int y;
};

struct Config
{
    int id;
    std::string name;
    std::vector<double> values;
    Point resolution;
    bool is_active;
};

int main()
{
    Config cfg{ 101, "SimulationConfig", { 0.5, 1.2, 3.14 }, { 1920, 1080 }, true };

    std::println("{:j}", cfg);
    // Output:
    // {"id":101,"name":"SimulationConfig","values":[0.5,1.2,3.14],
    // "resolution":{"x":1920,"y":1080},"is_active":true}

    std::println("{:pj}", cfg);
    /* Output:
    {
       "id": 101,
       "name": "SimulationConfig",
       "values": [
          0.5,
          1.2,
          3.14
       ],
       "resolution": {
          "x": 1920,
          "y": 1080
       },
       "is_active": true
    }
    */

    std::println("{:t}", cfg);
    /* Output:
    id = 101
    name = "SimulationConfig"
    values = [0.5, 1.2, 3.14]
    is_active = true
    [resolution]
    x = 1920
    y = 1080
    */
}
```

## Installation

### CMake FetchContent

You can easily include format_utils with its necessary dependencies in your CMake project:

```cmake
include(FetchContent)

FetchContent_Declare(
    format_utils
    GIT_REPOSITORY https://github.com/daleondev/format_utils.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(format_utils)

target_link_libraries(your_target PRIVATE format_utils::format_utils)
```

### Available CMake Configuration Options

| Option | Description | Default |
| :--- | :--- | :--- |
| `FMTU_ENABLE_JSON` | Enable JSON support via Glaze | `OFF` |
| `FMTU_ENABLE_TOML` | Enable TOML support via Glaze | `OFF` |
| `FMTU_ENABLE_YAML` | Enable YAML support via Glaze | `OFF` |
| `BUILD_SAMPLES` | Build sample executables | `ON` |
| `BUILD_TESTS` | Build unit tests | `ON` |

## Build Instructions

### Build the sample and tests:

```bash
# cmake --preset <compiler>-<build_type>-<platform>
cmake --preset clang-release-linux

# cmake --build --preset <compiler>-<build_type>-<platform>
cmake --build --preset clang-release-linux
```

### Run the sample:
```bash
# ./build/<compiler>-<build_type>-<platform>/samples/format_sample
./build/clang-release-linux/samples/format_sample
```

### Run the tests:
```bash
# ctest --preset <compiler>-<build_type>-<platform>
ctest --preset clang-release-linux
```

### Lint the project:

```bash
# cmake --preset tidy-<platform>
cmake --preset tidy-linux

# cmake --build --preset tidy-<platform>
cmake --build --preset tidy-linux --clean-first
```

### Available CMake Presets

| Preset Name | Description | Compiler |
| :--- | :--- | :--- |
| `debug` / `release` | System default compiler | Default |
| `gcc-debug` / `gcc-release` | Build using GCC | `g++` |
| `clang-debug` / `clang-release` | Build using Clang | `clang++` |
| `msvc-debug` / `msvc-release` | Build using MSVC | `cl` |
| `clang-debug-linux` / `clang-release-linux` | Build using Clang with libc++ | `clang++` (`-stdlib=libc++`) |
| `clang-debug-mingw` / `clang-release-mingw` | Build using Clang for MinGW | `clang++` (MinGW target) |
| `tidy-linux` / `tidy-mingw` | Static analysis with Clang-Tidy | `clang-tidy` |

## Compiler Support

This library is header-only and requires a C++23 compliant compiler. It is continuously tested on the following platforms:

- **Linux (Ubuntu)**:
  - GCC 14.2
  - Clang 19.1
- **Windows**:
  - MSVC 19.44 (Visual Studio 2022)
  - GCC 15.2 (MinGW-w64)
  - Clang 20.1 (MinGW-w64)

## Acknowledgements

**format_utils** is made possible by these incredible open-source projects:

- **[reflect](https://github.com/qlibs/reflect)**: Powerful C++20 static reflection library used for aggregate and enum metadata.
- **[Glaze](https://github.com/stephenberry/glaze)**: Extremely fast C++ library for JSON, YAML, and TOML serialization.
- **[Googletest](https://github.com/google/googletest)**: Industrial-strength testing framework.

For third-party license information, please see [THIRD-PARTY-NOTICES.txt](THIRD-PARTY-NOTICES.txt).