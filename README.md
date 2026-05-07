# reflection

## reflection value

```cpp
struct Point
{
    int x;
    int y;
};

constexpr std::meta::info r_Point = ^^Point;

using MyPoint = [:r_Point:];
MyPoint p1{1, 2};
typename[:r_Point:] p2{3, 4};
```

### name

```cpp
std::string_view name_Point = std::meta::identifier_of(r_Point);
```

### members

```cpp
consteval std::meta::info member_number(int n) {
    auto ctx = std::meta::access_context::current();
    return std::meta::nonstatic_data_members_of(^^Point, ctx)[n];
}

p2.[:member_number(0):] = 23;
```