# mirror

mirror is an experimental C++ reflection-based serialization library.

It turns C++ objects into a typed intermediate tree, then writes that tree to formats such as JSON and YAML. The same tree can be read back and deserialized into C++ objects.

The core idea is simple:

```cpp
const mirror::value tree = mirror::serialize(object);
const std::string json = mirror::json::write(tree);

auto parsed = mirror::deserialize<MyType>(mirror::json::read(json));
```

Mirror uses C++ reflection to walk struct/class fields automatically. For supported primitive and standard-library types, users do not need to write per-type serializers.

## Status

Mirror is currently experimental and targets compilers with C++ reflection support.

The current project is built with:

- C++26
- `std::meta` reflection
- GCC reflection flag: `-freflection`

This is not portable C++20/23 code today. It is intended for modern compiler experiments around reflection-driven serialization.

## Features

- Automatic field iteration for reflected structs/classes
- Recursive serialization and deserialization
- Format-independent intermediate tree: `mirror::value`
- JSON backend using `nlohmann/json`
- YAML backend using `yaml-cpp`
- Plain JSON/YAML primitive output driven by the target C++ type during deserialization
- Return-by-value deserialization
- In-place deserialization

## Supported Types

Mirror currently supports:

- reflected structs/classes
- `bool`
- fixed-width and native signed integers
- fixed-width and native unsigned integers
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

The test suite covers representative examples of these categories.

## Quick Start

Define a regular aggregate type:

```cpp
struct Point
{
    std::int32_t x;
    std::int32_t y;
};

struct User
{
    std::string            name;
    std::int32_t           age;
    Point                  home;
    std::unique_ptr<Point> point;
};
```

Include Mirror headers:

```cpp
#include <mirror/json.hpp>
#include <mirror/mirror.hpp>
#include <mirror/yaml.hpp>
```

Serialize to JSON:

```cpp
Point point{23, 67};
User user{"fatih", 23, point, std::make_unique<Point>(point)};

const mirror::value tree = mirror::serialize(user);
const std::string document = mirror::json::write(tree);
```

Deserialize from JSON:

```cpp
const User parsed = mirror::deserialize<User>(mirror::json::read(document));
```

Serialize to YAML:

```cpp
const std::string yaml = mirror::yaml::write(mirror::serialize(user));
```

Deserialize from YAML:

```cpp
const User parsed = mirror::deserialize<User>(mirror::yaml::read(yaml));
```

## API

### Core Serialization

```cpp
template <typename Type>
mirror::value mirror::serialize(const Type& input);
```

Creates a `mirror::value` tree from a C++ object.

### In-Place Deserialization

```cpp
template <typename Type>
void mirror::deserialize(const mirror::value& input, Type& output);
```

Writes deserialized data into an existing object.

This is required for raw pointer fields when the pointer must refer to caller-owned storage.

### Return-By-Value Deserialization

```cpp
template <typename Type>
Type mirror::deserialize(const mirror::value& input);
```

Default-constructs `Type`, fills it, and returns it.

This works naturally for values and owning smart pointers. It intentionally rejects null raw pointers because Mirror cannot infer ownership.

### JSON

```cpp
std::string mirror::json::write(const mirror::value& input);
mirror::value mirror::json::read(std::string_view input);
```

### YAML

```cpp
std::string mirror::yaml::write(const mirror::value& input);
mirror::value mirror::yaml::read(std::string_view input);
```

## Intermediate Tree

Mirror uses `mirror::value` as its format-independent representation.

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

This keeps the serialization engine independent of JSON, YAML, or any future backend.

## Primitive Values

JSON and YAML do not preserve all C++ scalar information by default. For example, JSON has numbers, but it does not distinguish:

- `std::int8_t`
- `std::uint8_t`
- `std::int32_t`
- `std::uint64_t`
- `float`
- `double`

Mirror-produced JSON/YAML intentionally emits primitive values as plain JSON/YAML scalars:

```json
-5
```

The target C++ type controls final deserialization. When Mirror reads plain external JSON/YAML numbers into the intermediate tree without a reflected destination type, it defaults to:

- signed integer: 64-bit
- unsigned integer: 64-bit
- floating point: 64-bit

During typed deserialization, Mirror rejects numeric values that do not fit in the destination C++ type.

## Pointer Semantics

Pointer wrappers are not stored in the document.

These fields serialize as the pointed value or null:

```cpp
Point* point;
std::unique_ptr<Point> point;
std::shared_ptr<Point> point;
```

The destination C++ type controls reconstruction.

### Smart Pointers

Owning smart pointers are the recommended way to represent optional owned objects:

```cpp
std::unique_ptr<Point> point;
std::shared_ptr<Point> point;
```

During deserialization, Mirror allocates the pointee when needed.

### Raw Pointers

Raw pointers are allowed only when the caller provides valid storage for deserialization:

```cpp
Point storage{};
User user{};
user.point = &storage;

mirror::deserialize(tree, user);
```

Return-by-value deserialization into a type with a non-null raw pointer payload is rejected if the raw pointer field is null:

```txt
cannot deserialize into a null raw pointer; use an owning smart pointer or provide valid storage
```

Mirror does not invent ownership for raw pointers.

### Aliasing

Mirror currently serializes pointer pointees as values. It does not preserve pointer identity or aliasing.

This does not round-trip as an alias:

```cpp
user.point = &user.home;
```

Supporting that would require an object ID/reference layer in the intermediate tree.

## Type Information

Reflected objects include a `_mirror_type` field:

```json
{
  "_mirror_type": "Point",
  "x": 23,
  "y": 67
}
```

During deserialization, if `_mirror_type` is present, Mirror checks it against the reflected destination type name.

## Containers

Sequence containers serialize as arrays:

```cpp
std::vector<std::string> tags{"admin", "debug"};
```

Map-like containers serialize as arrays of key/value entries so non-string keys can be supported:

```json
[
  {
    "_mirror_type": "map_entry",
    "key": "...",
    "value": "..."
  }
]
```

Tuple-like types serialize as arrays in tuple order.

## Build

The project uses CMake and FetchContent.

Dependencies:

- `nlohmann/json`
- `yaml-cpp`
- `Catch2`

The optional demo executable also uses `spdlog`.

Configure and build:

```powershell
cmake -S . -B build -G Ninja
cmake --build build --config Debug
```

Use Mirror from another CMake project with FetchContent:

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

Then include the umbrella header and the backend headers you use:

```cpp
#include <mirror/mirror.hpp>
#include <mirror/json.hpp>
// or
#include <mirror/yaml.hpp>
```

The demo executable is disabled by default. Enable it with:

```powershell
cmake -S . -B build -G Ninja -DMIRROR_BUILD_DEMO=ON
```

The demo source lives in `examples/demo.cpp`. When enabled, the executable is emitted under:

```text
runtime/
```

## Tests

Tests are built as:

```text
mirror_tests
```

Run through CTest:

```powershell
ctest --test-dir build --output-on-failure
```

Or run the executable directly from `runtime/` after building.

The tests cover:

- primitive scalar round trips
- plain primitive JSON/YAML output
- JSON and YAML round trips
- arrays and containers
- maps and unordered maps
- tuple-like types
- optionals
- variants
- smart pointers
- raw pointer failure and in-place deserialization
- complex nested documents

The test suite is split by area:

- `Value.test.cpp`
- `FormatBackends.test.cpp`
- `SerializationRoundTrip.test.cpp`
- `PointerSerialization.test.cpp`
- `DeserializationErrors.test.cpp`
- `CustomAdapter.test.cpp`

## Headers

Current public headers:

```cpp
#include <mirror/mirror.hpp>
#include <mirror/json.hpp>
#include <mirror/yaml.hpp>
```

Internal serialization is routed through `src/detail/dispatch.hpp`. Built-in semantic adapters live under `src/adapters/`. Shared concepts and numeric utilities live under `src/detail/`.

Current source layout:

```text
include/
  mirror/
    mirror.hpp
    json.hpp
    yaml.hpp

src/
  adapter.hpp
  serialize.hpp
  deserialize.hpp
  value.hpp/.cpp
  reflect.hpp

  adapters/
    scalar.hpp
    pointer.hpp
    optional.hpp
    variant.hpp
    range.hpp
    map.hpp
    tuple.hpp
    object.hpp

  detail/
    dispatch.hpp
    concepts.hpp
    utils.hpp

  backends/
    json.hpp/.cpp
    yaml.hpp/.cpp
```

## Adapters

Mirror dispatches serialization through adapters. Built-in adapters handle standard semantic categories such as scalars, pointers, optionals, variants, ranges, maps, tuples, and reflected objects.

Custom types can override dispatch by specializing `mirror::adapter<T>`:

```cpp
template <>
struct mirror::adapter<MyType>
{
    static mirror::value serialize(const MyType& input);
    static void deserialize(const mirror::value& input, MyType& output);
};
```

Reflection is the fallback for regular user data objects when no custom or built-in semantic adapter applies.

## Design Notes

Mirror intentionally separates the engine from output formats:

```text
C++ object
   -> mirror::value tree
      -> JSON/YAML/etc.
```

This makes it possible to add more backends later without changing field traversal or C++ type handling.

Future streaming support can be added by replacing the tree backend with a streaming intermediate writer/reader. The current implementation is tree-first because it is easier to reason about, test, and adapt to multiple formats.

## Current Limitations

- Requires experimental C++ reflection support.
- Does not preserve pointer identity or aliasing.
- Does not support polymorphic dynamic type dispatch yet.
- Does not serialize private fields unless reflection access rules allow it.
- Does not currently provide a binary/network wire format, only typed trees emitted through JSON/YAML.
- Plain external JSON/YAML numbers have default widths because those formats do not carry C++ scalar sizes.

## Roadmap Ideas

- Binary backend with fixed-size network encoding
- Object ID/reference support for pointer aliasing
- Polymorphic serialization
- Custom per-type adapters
- Versioned schemas and migration hooks
- Better diagnostics with field paths
- Streaming reader/writer API
