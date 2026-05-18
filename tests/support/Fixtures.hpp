#pragma once

#include <catch2/catch_test_macros.hpp>

#include <mirror/json.hpp>
#include <mirror/mirror.hpp>
#include <mirror/yaml.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>

namespace mirror::test
{

enum class Mode : std::uint16_t
{
    idle = 1,
    active = 42
};

struct Point
{
    std::int32_t x = 0;
    std::int32_t y = 0;
};

struct ScalarTypes
{
    bool           boolean = false;
    char           character = '\0';
    char16_t       wide_character = 0;
    std::int8_t    i8 = 0;
    std::uint8_t   u8 = 0;
    std::int16_t   i16 = 0;
    std::uint16_t  u16 = 0;
    std::int32_t   i32 = 0;
    std::uint32_t  u32 = 0;
    std::int64_t   i64 = 0;
    std::uint64_t  u64 = 0;
    float          f32 = 0.0f;
    double         f64 = 0.0;
    Mode           mode = Mode::idle;
    std::byte      byte = std::byte{0};
    std::nullptr_t null_value = nullptr;
    std::monostate empty_state;
    std::string    text;
};

struct ContainerTypes
{
    std::array<std::int16_t, 3>                 fixed;
    std::int32_t                                c_array[3]{};
    std::vector<std::string>                    vector;
    std::list<std::int32_t>                     list;
    std::set<std::int32_t>                      set;
    std::map<std::string, std::int32_t>         map;
    std::unordered_map<std::string, std::int32_t> unordered_map;
    std::pair<std::int32_t, std::string>        pair;
    std::tuple<std::int32_t, std::string, bool> tuple;
};

struct OptionalVariantTypes
{
    std::optional<std::uint32_t>             present;
    std::optional<std::string>              empty;
    std::variant<std::int32_t, std::string> variant;
    std::variant<std::monostate, Point>     nullable_variant;
};

struct PointerTypes
{
    std::unique_ptr<Point> unique;
    std::shared_ptr<Point> shared;
    Point*                 raw = nullptr;
};

struct ComplexDocument
{
    std::string                                                           id;
    ScalarTypes                                                           scalars;
    ContainerTypes                                                        containers;
    OptionalVariantTypes                                                  optional_variant;
    std::unique_ptr<Point>                                                origin;
    std::vector<std::map<std::string, std::variant<std::int32_t, std::string>>> rows;
};

inline ScalarTypes make_scalars()
{
    return {
        true,
        'x',
        u'z',
        static_cast<std::int8_t>(-8),
        static_cast<std::uint8_t>(250),
        -1234,
        54321,
        -12345678,
        3456789012u,
        -1234567890123ll,
        9'000'000'000ull,
        3.25f,
        99.125,
        Mode::active,
        std::byte{200},
        nullptr,
        std::monostate{},
        "hello"
    };
}

inline ContainerTypes make_containers()
{
    return {
        {1, 2, 3},
        {4, 5, 6},
        {"alpha", "beta"},
        {7, 8, 9},
        {10, 11, 12},
        {{"one", 1}, {"two", 2}},
        {{"three", 3}, {"four", 4}},
        {13, "pair"},
        {14, "tuple", true}
    };
}

inline OptionalVariantTypes make_optional_variant()
{
    return {
        77u,
        std::nullopt,
        std::string{"variant"},
        Point{15, 16}
    };
}

inline ComplexDocument make_complex()
{
    return {
        "document",
        make_scalars(),
        make_containers(),
        make_optional_variant(),
        std::make_unique<Point>(Point{17, 18}),
        {
            {{"a", 1}, {"b", std::string{"two"}}},
            {{"c", 3}, {"d", std::string{"four"}}}
        }
    };
}

inline void require_equal(const Point& left, const Point& right)
{
    REQUIRE(left.x == right.x);
    REQUIRE(left.y == right.y);
}

inline void require_equal(const ScalarTypes& left, const ScalarTypes& right)
{
    REQUIRE(left.boolean == right.boolean);
    REQUIRE(left.character == right.character);
    REQUIRE(left.wide_character == right.wide_character);
    REQUIRE(left.i8 == right.i8);
    REQUIRE(left.u8 == right.u8);
    REQUIRE(left.i16 == right.i16);
    REQUIRE(left.u16 == right.u16);
    REQUIRE(left.i32 == right.i32);
    REQUIRE(left.u32 == right.u32);
    REQUIRE(left.i64 == right.i64);
    REQUIRE(left.u64 == right.u64);
    REQUIRE(std::fabs(left.f32 - right.f32) < 0.0001f);
    REQUIRE(std::fabs(left.f64 - right.f64) < 0.0000001);
    REQUIRE(left.mode == right.mode);
    REQUIRE(left.byte == right.byte);
    REQUIRE(left.null_value == right.null_value);
    REQUIRE(left.empty_state == right.empty_state);
    REQUIRE(left.text == right.text);
}

inline void require_equal(const ContainerTypes& left, const ContainerTypes& right)
{
    REQUIRE(left.fixed == right.fixed);
    REQUIRE(std::ranges::equal(left.c_array, right.c_array));
    REQUIRE(left.vector == right.vector);
    REQUIRE(left.list == right.list);
    REQUIRE(left.set == right.set);
    REQUIRE(left.map == right.map);
    REQUIRE(left.unordered_map == right.unordered_map);
    REQUIRE(left.pair == right.pair);
    REQUIRE(left.tuple == right.tuple);
}

inline void require_equal(const OptionalVariantTypes& left, const OptionalVariantTypes& right)
{
    REQUIRE(left.present == right.present);
    REQUIRE(left.empty == right.empty);
    REQUIRE(left.variant == right.variant);
    REQUIRE(left.nullable_variant.index() == right.nullable_variant.index());
    require_equal(std::get<Point>(left.nullable_variant), std::get<Point>(right.nullable_variant));
}

inline void require_equal(const ComplexDocument& left, const ComplexDocument& right)
{
    REQUIRE(left.id == right.id);
    require_equal(left.scalars, right.scalars);
    require_equal(left.containers, right.containers);
    require_equal(left.optional_variant, right.optional_variant);
    REQUIRE(left.origin != nullptr);
    REQUIRE(right.origin != nullptr);
    require_equal(*left.origin, *right.origin);
    REQUIRE(left.rows == right.rows);
}

inline void require_throws_message(auto&& function, std::string_view expected)
{
    bool threw_expected_error = false;
    try
    {
        function();
    }
    catch(const std::runtime_error& error)
    {
        threw_expected_error = std::string_view{error.what()} == expected;
    }
    REQUIRE(threw_expected_error);
}

template <typename Type>
Type json_round_trip(const Type& input)
{
    return mirror::deserialize<Type>(mirror::json::read(mirror::json::write(mirror::serialize(input))));
}

template <typename Type>
Type yaml_round_trip(const Type& input)
{
    return mirror::deserialize<Type>(mirror::yaml::read(mirror::yaml::write(mirror::serialize(input))));
}

} // namespace mirror::test
