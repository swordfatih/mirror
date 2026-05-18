# mirror

Reflection-based serialization for modern C++ experiments.

Mirror turns C++ objects into a typed intermediate tree, then writes that tree as JSON, YAML, or compact binary. The same tree can be read back and deserialized into C++ objects.

```cpp
#include <mirror/formats/binary.hpp>
#include <mirror/formats/json.hpp>
#include <mirror/formats/yaml.hpp>
#include <mirror/mirror.hpp>

#include <cstdint>
#include <memory>
#include <string>

struct Point
{
    std::int32_t x = 0;
    std::int32_t y = 0;
};

struct User
{
    std::string            name;
    std::int32_t           age = 0;
    Point                  home;
    std::unique_ptr<Point> cursor;
};

int main()
{
    User user{
        .name = "username",
        .age = 23,
        .home = {10, 20},
        .cursor = std::make_unique<Point>(Point{30, 40})
    };

    const mirror::value tree = mirror::serialize(user);
    const std::string   text = mirror::json::write(tree);

    const User copy = mirror::deserialize<User>(mirror::json::read(text));
}
```

## Status

Mirror is experimental. It targets compilers with C++ reflection support and is not portable C++20/23 code today.

Current requirements:

- C++26
- `std::meta`
- GCC reflection support through `-freflection`
- CMake 4.3.2 or newer for this project configuration

## Installation

Mirror is designed to be consumed with CMake `FetchContent`.

```cmake
include(FetchContent)

FetchContent_Declare(
    mirror
    GIT_REPOSITORY https://github.com/swordfatih/mirror.git
    GIT_TAG main
)
FetchContent_MakeAvailable(mirror)

target_link_libraries(your_target PRIVATE mirror::mirror)
```

Then include the core header and the format backend headers you use:

```cpp
#include <mirror/mirror.hpp>
#include <mirror/formats/binary.hpp>
#include <mirror/formats/json.hpp>
#include <mirror/formats/yaml.hpp>
```

When Mirror is embedded through `FetchContent`, tests are disabled by default. When this repository is configured as the top-level project, tests are enabled by default.

## Quick Start

### Serialize To JSON

```cpp
const mirror::value tree = mirror::serialize(object);
const std::string json = mirror::json::write(tree);
```

### Deserialize From JSON

```cpp
const auto object = mirror::deserialize<MyType>(mirror::json::read(json));
```

### Serialize To YAML

```cpp
const std::string yaml = mirror::yaml::write(mirror::serialize(object));
```

### Deserialize From YAML

```cpp
const auto object = mirror::deserialize<MyType>(mirror::yaml::read(yaml));
```

### Serialize To Binary

The binary backend returns a `std::string` containing bytes. For TCP or file storage, send or store it with an explicit length prefix or another framing scheme.

```cpp
const std::string bytes = mirror::binary::write(mirror::serialize(object));
```

### Deserialize From Binary

```cpp
const auto object = mirror::deserialize<MyType>(mirror::binary::read(bytes));
```

### Deserialize Into An Existing Object

Use in-place deserialization when the destination already owns storage, especially for raw pointer fields.

```cpp
Point storage{};
User user{};
user.cursor = nullptr;

mirror::deserialize(tree, user);
```

## What Mirror Supports

Mirror currently supports:

- reflected structs/classes
- `bool`
- signed and unsigned integer types
- floating point types
- `char`, `wchar_t`, `char8_t`, `char16_t`, `char32_t`
- `std::byte`
- `std::nullptr_t`
- `std::monostate`
- enums, serialized through their underlying type
- `std::string`
- raw pointers, with restrictions
- `std::unique_ptr<T>`
- `std::shared_ptr<T>`
- `std::optional<T>`
- `std::variant<Ts...>`
- C arrays
- `std::array<T, N>`
- sequence containers with `begin/end` and `push_back` or `insert`
- map-like containers with `key_type` and `mapped_type`
- tuple-like types such as `std::pair` and `std::tuple`

## Public API

### Core

```cpp
template <typename Type>
mirror::value mirror::serialize(const Type& input);

template <typename Type>
Type mirror::deserialize(const mirror::value& input);

template <typename Type>
void mirror::deserialize(const mirror::value& input, Type& output);
```

### JSON Format

```cpp
std::string mirror::json::write(const mirror::value& input);
mirror::value mirror::json::read(std::string_view input);
```

### YAML Format

```cpp
std::string mirror::yaml::write(const mirror::value& input);
mirror::value mirror::yaml::read(std::string_view input);
```

### Binary Format

```cpp
std::string mirror::binary::write(const mirror::value& input);
mirror::value mirror::binary::read(std::string_view input);
```

The binary format is compact and recursive:

- one byte value kind tags
- variable-length unsigned integers for sizes and integer payloads
- zig-zag encoding for signed integers
- little-endian IEEE payloads for 32-bit and 64-bit floating point values
- length-prefixed strings, field names, arrays, and objects
- a short magic/version prefix for format detection

## Intermediate Tree

Mirror serializes through `mirror::value`, a format-independent tree.

The tree can represent:

- object
- array
- string
- character
- signed integer
- unsigned integer
- floating point
- boolean
- null

Objects store named fields. Arrays store ordered elements. Primitive scalars store their text representation and, where relevant, their bit width.

This keeps object traversal independent from JSON, YAML, binary, or future formats.

## Type Information

Reflected objects include a `_mirror_type` field:

```json
{
  "_mirror_type": "Point",
  "x": 10,
  "y": 20
}
```

During deserialization, Mirror checks `_mirror_type` when it is present.

All reflected fields are required during object deserialization. Missing fields are reported as errors instead of being silently default-initialized.

## Primitive Values

JSON and YAML do not preserve all C++ scalar details by themselves. For example, they do not distinguish `std::int8_t`, `std::uint32_t`, `float`, and `double` in the same way C++ does.

Mirror-produced JSON/YAML emits plain values:

```json
-5
```

The destination C++ type controls final deserialization. When reading external JSON/YAML into `mirror::value`, numbers default to 64-bit integer or floating-point nodes. Typed deserialization then rejects values that do not fit in the target C++ type.

The binary backend keeps numeric payloads in binary form where possible while preserving Mirror's typed tree shape.

## SQL CRUD Extension

Mirror includes a small header-only SQL CRUD statement builder. It uses `mirror::meta` to inspect model fields and produces SQL text plus bind values. It does not open database connections or execute statements.

```cpp
#include <mirror/extensions/sql.hpp>

struct User
{
    std::uint64_t id = 0;
    std::string name;
};

template <>
struct mirror::sql::table<User>
{
    static constexpr std::string_view name = "users";
    static constexpr std::string_view primary_key = "id";
};

User user{.id = 1, .name = "username"};

auto insert = mirror::sql::insert(user);
auto update = mirror::sql::update(user);
auto select = mirror::sql::select_by_id<User>(1);
auto remove = mirror::sql::delete_by_id<User>(1);
```

Example generated statement:

```sql
INSERT INTO users (id, name) VALUES (?, ?)
```

`mirror::sql::statement` stores the SQL text and a vector of bind values. Database-specific execution belongs in a thin adapter for SQLite, PostgreSQL, MySQL, or another driver.

## Pointers

Pointer wrappers are not stored in the document. These all serialize as the pointed value or `null`:

```cpp
Point* point;
std::unique_ptr<Point> point;
std::shared_ptr<Point> point;
```

Smart pointers are the recommended way to represent optional owned objects. During deserialization, Mirror allocates the pointee when needed.

Raw pointers are supported only when the caller provides storage:

```cpp
Point storage{};
Point* output = &storage;

mirror::deserialize(tree, output);
```

Mirror does not preserve pointer identity or aliasing. If two pointers refer to the same object before serialization, they deserialize as independent values.

## Containers

Sequence containers serialize as arrays:

```json
["admin", "debug"]
```

Map-like containers serialize as arrays of key/value entries, which allows non-string keys:

```json
[
  {
    "_mirror_type": "map_entry",
    "key": 1,
    "value": "one"
  }
]
```

Tuple-like types serialize as arrays in tuple order.

## Custom Adapters

Specialize `mirror::adapter<T>` to override built-in dispatch for a type.

```cpp
template <>
struct mirror::adapter<MyType>
{
    static mirror::value serialize(const MyType& input);
    static void deserialize(const mirror::value& input, MyType& output);
};
```

## Build This Repository

Configure and build:

```powershell
cmake -S . -B build -G Ninja
cmake --build build --config Debug
```

Build the demo:

```powershell
cmake -S . -B build -G Ninja -DMIRROR_BUILD_DEMO=ON
cmake --build build --config Debug
```

The repository also includes [a tiny TCP-style binary framing example](examples/tcp_binary.cpp). It uses a 4-byte big-endian length prefix followed by `mirror::binary` payload bytes.

There is also [a small SQL CRUD statement example](examples/sql_crud.cpp). It generates `INSERT`, `UPDATE`, `SELECT ... WHERE id = ?`, and `DELETE ... WHERE id = ?` statements with bind values. It does not connect to a database.

Run tests:

```powershell
ctest --test-dir build --output-on-failure
```

Useful CMake options:

- `MIRROR_BUILD_TESTS`: build tests. Defaults to `ON` only when Mirror is the top-level project.
- `MIRROR_BUILD_DEMO`: build the demo executable. Defaults to `OFF`.

## Project Layout

```text
include/
  mirror/
    mirror.hpp
    meta.hpp

    meta/
      reflect.hpp
      field.hpp

    codec/
      value.hpp
      adapter.hpp
      serialize.hpp
      deserialize.hpp
      dispatch.hpp
      concepts.hpp
      scalar_utils.hpp
      value_utils.hpp
      adapters/
        scalar.hpp
        pointer.hpp
        optional.hpp
        variant.hpp
        range.hpp
        map.hpp
        tuple.hpp
        object.hpp

    formats/
      binary.hpp
      json.hpp
      yaml.hpp

    extensions/
      sql.hpp
      sql/
        adapter.hpp
        bind.hpp
        crud.hpp
        statement.hpp
        table.hpp

src/
  value.cpp
  formats/
    binary.cpp
    json.cpp
    yaml.cpp
```

The layers are intentionally separated:

- `meta`: reflected type names and field iteration
- `codec`: type-category detection, `mirror::value`, and C++ object `<-> mirror::value`
- `formats`: `mirror::value <->` JSON, YAML, or binary
- `extensions/sql`: SQL CRUD statement generation using `mirror::meta`

## Limitations

- Requires experimental C++ reflection support.
- Does not preserve pointer identity or aliasing.
- Does not support polymorphic dynamic type dispatch.
- Does not serialize private fields unless reflection access rules allow it.
- Provides tree-level binary serialization, but no built-in TCP framing or schema negotiation.
- External JSON/YAML numbers use default widths because those formats do not carry C++ scalar sizes.

## Roadmap Ideas

- Binary backend with fixed-size network encoding
- Object ID/reference support for pointer aliasing
- Polymorphic serialization
- Versioned schemas and migration hooks
- Better diagnostics with field paths
- Streaming reader/writer API
