#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <meta>

import std;
import mirror.json;
import mirror.reflect;
import mirror.serialization;
import mirror.value;
import mirror.yaml;

namespace
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
    bool          boolean = false;
    char          character = '\0';
    char16_t      wide_character = 0;
    std::int8_t   i8 = 0;
    std::uint8_t  u8 = 0;
    std::int16_t  i16 = 0;
    std::uint16_t u16 = 0;
    std::int32_t  i32 = 0;
    std::uint32_t u32 = 0;
    std::int64_t  i64 = 0;
    std::uint64_t u64 = 0;
    float         f32 = 0.0f;
    double        f64 = 0.0;
    Mode          mode = Mode::idle;
    std::byte     byte = std::byte{0};
    std::nullptr_t null_value = nullptr;
    std::monostate empty_state;
    std::string   text;
};

struct ContainerTypes
{
    std::array<std::int16_t, 3>         fixed;
    std::int32_t                        c_array[3]{};
    std::vector<std::string>            vector;
    std::list<std::int32_t>             list;
    std::set<std::int32_t>              set;
    std::map<std::string, std::int32_t> map;
    std::unordered_map<std::string, std::int32_t> unordered_map;
    std::pair<std::int32_t, std::string> pair;
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
    std::string                                      id;
    ScalarTypes                                      scalars;
    ContainerTypes                                   containers;
    OptionalVariantTypes                             optional_variant;
    std::unique_ptr<Point>                           origin;
    std::vector<std::map<std::string, std::variant<std::int32_t, std::string>>> rows;
};

ScalarTypes make_scalars()
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

ContainerTypes make_containers()
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

OptionalVariantTypes make_optional_variant()
{
    return {
        77u,
        std::nullopt,
        std::string{"variant"},
        Point{15, 16}
    };
}

ComplexDocument make_complex()
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

void require_equal(const Point& left, const Point& right)
{
    REQUIRE(left.x == right.x);
    REQUIRE(left.y == right.y);
}

void require_equal(const ScalarTypes& left, const ScalarTypes& right)
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

void require_equal(const ContainerTypes& left, const ContainerTypes& right)
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

void require_equal(const OptionalVariantTypes& left, const OptionalVariantTypes& right)
{
    REQUIRE(left.present == right.present);
    REQUIRE(left.empty == right.empty);
    REQUIRE(left.variant == right.variant);
    REQUIRE(left.nullable_variant.index() == right.nullable_variant.index());
    require_equal(std::get<Point>(left.nullable_variant), std::get<Point>(right.nullable_variant));
}

void require_equal(const ComplexDocument& left, const ComplexDocument& right)
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

void require_throws_message(auto&& function, std::string_view expected)
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

} // namespace

TEST_CASE("scalar primitives preserve values through JSON and YAML", "[serialization][scalars]")
{
    const auto input = make_scalars();

    require_equal(input, json_round_trip(input));
    require_equal(input, yaml_round_trip(input));
}

TEST_CASE("typed primitive metadata is emitted with the preferred key", "[serialization][metadata]")
{
    const auto document = mirror::json::write(mirror::serialize(std::int32_t{-5}));

    REQUIRE(document.find("_primitive") != std::string::npos);
    REQUIRE(document.find("_mirror_kind") == std::string::npos);
    REQUIRE(document.find("\"bits\":32") != std::string::npos);
}

TEST_CASE("mirror value factories and lookup behave predictably", "[value]")
{
    auto object = mirror::value::object("Point");
    object.fields.emplace_back("x", mirror::value::signed_integer("10", 32));

    REQUIRE(object.type == mirror::value::kind::object);
    REQUIRE(object.find("_type") != nullptr);
    REQUIRE(object.find("x") != nullptr);
    REQUIRE(object.find("missing") == nullptr);
    REQUIRE_THROWS_AS(mirror::value::string("text").find("x"), std::runtime_error);

    auto array = mirror::value::array();
    array.elements.emplace_back(mirror::value::boolean_value(true));
    REQUIRE(array.type == mirror::value::kind::array);
    REQUIRE(array.elements.size() == 1);

    REQUIRE(mirror::value::character("65", 8).bits == 8);
    REQUIRE(mirror::value::unsigned_integer("1", 32).bits == 32);
    REQUIRE(mirror::value::floating_point("1.5", 64).bits == 64);
}

TEST_CASE("containers and tuple-like types round-trip", "[serialization][containers]")
{
    const auto input = make_containers();

    require_equal(input, json_round_trip(input));
    require_equal(input, yaml_round_trip(input));
}

TEST_CASE("optional and variant values round-trip", "[serialization][optional][variant]")
{
    const auto input = make_optional_variant();

    require_equal(input, json_round_trip(input));
    require_equal(input, yaml_round_trip(input));
}

TEST_CASE("owning smart pointers round-trip and null smart pointers stay null", "[serialization][pointers]")
{
    PointerTypes input;
    input.unique = std::make_unique<Point>(Point{1, 2});
    input.shared = std::make_shared<Point>(Point{3, 4});

    const auto output = json_round_trip(input);

    REQUIRE(output.unique != nullptr);
    REQUIRE(output.shared != nullptr);
    REQUIRE(output.raw == nullptr);
    require_equal(*input.unique, *output.unique);
    require_equal(*input.shared, *output.shared);

    PointerTypes null_input;
    const auto null_output = json_round_trip(null_input);
    REQUIRE(null_output.unique == nullptr);
    REQUIRE(null_output.shared == nullptr);
    REQUIRE(null_output.raw == nullptr);
}

TEST_CASE("raw pointers require caller-provided storage during deserialization", "[serialization][pointers]")
{
    Point source_point{5, 6};
    PointerTypes input;
    input.raw = &source_point;

    const auto tree = mirror::serialize(input);

    require_throws_message([&] {
        static_cast<void>(mirror::deserialize<PointerTypes>(tree));
    }, "cannot deserialize into a null raw pointer; use an owning smart pointer or provide valid storage");

    Point storage{};
    PointerTypes output;
    output.raw = &storage;

    mirror::deserialize(tree, output);
    REQUIRE(output.raw == &storage);
    require_equal(source_point, *output.raw);
}

TEST_CASE("external JSON values use default primitive widths", "[json][external]")
{
    auto unsigned_value = mirror::json::read("18446744073709551615");
    REQUIRE(unsigned_value.type == mirror::value::kind::unsigned_integer);
    REQUIRE(unsigned_value.bits == 64);
    REQUIRE(unsigned_value.text == "18446744073709551615");

    auto signed_value = mirror::json::read("-42");
    REQUIRE(signed_value.type == mirror::value::kind::signed_integer);
    REQUIRE(signed_value.bits == 64);
    REQUIRE(signed_value.text == "-42");

    auto floating_value = mirror::json::read("3.5");
    REQUIRE(floating_value.type == mirror::value::kind::floating_point);
    REQUIRE(floating_value.bits == 64);

    REQUIRE(mirror::json::read("true").type == mirror::value::kind::boolean);
    REQUIRE(mirror::json::read("null").type == mirror::value::kind::null);
    REQUIRE(mirror::json::read(R"("text")").type == mirror::value::kind::string);
    REQUIRE(mirror::json::read(R"([1,"two",false])").elements.size() == 3);

    require_throws_message([] {
        static_cast<void>(mirror::json::read(R"({"_primitive":"bad","bits":32,"value":"1"})"));
    }, "unknown mirror scalar kind");
}

TEST_CASE("external YAML values use default primitive widths", "[yaml][external]")
{
    auto signed_value = mirror::yaml::read("-42");
    REQUIRE(signed_value.type == mirror::value::kind::signed_integer);
    REQUIRE(signed_value.bits == 64);
    REQUIRE(signed_value.text == "-42");

    auto floating_value = mirror::yaml::read("3.5");
    REQUIRE(floating_value.type == mirror::value::kind::floating_point);
    REQUIRE(floating_value.bits == 64);

    REQUIRE(mirror::yaml::read("true").type == mirror::value::kind::boolean);
    REQUIRE(mirror::yaml::read("plain-text").type == mirror::value::kind::string);
    REQUIRE(mirror::yaml::read("- one\n- two\n").elements.size() == 2);

    require_throws_message([] {
        static_cast<void>(mirror::yaml::read("_primitive: bad\nbits: 32\nvalue: 1\n"));
    }, "unknown mirror scalar kind");
}

TEST_CASE("deserialization reports scalar kind mismatches", "[serialization][errors]")
{
    require_throws_message([] {
        static_cast<void>(mirror::deserialize<bool>(mirror::value::string("true")));
    }, "unexpected value kind");

    require_throws_message([] {
        static_cast<void>(mirror::deserialize<std::int32_t>(mirror::value::string("1")));
    }, "expected integer");

    require_throws_message([] {
        static_cast<void>(mirror::deserialize<std::uint32_t>(mirror::value::string("1")));
    }, "expected integer");

    require_throws_message([] {
        static_cast<void>(mirror::deserialize<float>(mirror::value::signed_integer("1", 32)));
    }, "unexpected value kind");

    require_throws_message([] {
        static_cast<void>(mirror::deserialize<std::string>(mirror::value::signed_integer("1", 32)));
    }, "unexpected value kind");

    require_throws_message([] {
        static_cast<void>(mirror::deserialize<char>(mirror::value::string("a")));
    }, "unexpected value kind");

    require_throws_message([] {
        static_cast<void>(mirror::deserialize<std::byte>(mirror::value::string("1")));
    }, "unexpected value kind");

    require_throws_message([] {
        static_cast<void>(mirror::deserialize<std::nullptr_t>(mirror::value::string("null")));
    }, "unexpected value kind");

    require_throws_message([] {
        static_cast<void>(mirror::deserialize<std::monostate>(mirror::value::string("null")));
    }, "unexpected value kind");
}

TEST_CASE("deserialization reports structural mismatches", "[serialization][errors]")
{
    require_throws_message([] {
        auto object = mirror::value::object("OtherPoint");
        object.fields.emplace_back("x", mirror::value::signed_integer("1", 32));
        object.fields.emplace_back("y", mirror::value::signed_integer("2", 32));
        static_cast<void>(mirror::deserialize<Point>(object));
    }, "unexpected object type");

    require_throws_message([] {
        static_cast<void>(mirror::deserialize<std::array<std::int32_t, 2>>(mirror::value::array()));
    }, "array element count mismatch");

    require_throws_message([] {
        mirror::value array = mirror::value::array();
        array.elements.emplace_back(mirror::value::signed_integer("1", 32));
        std::int32_t output[2]{};
        mirror::deserialize(array, output);
    }, "array element count mismatch");

    require_throws_message([] {
        mirror::value array = mirror::value::array();
        array.elements.emplace_back(mirror::value::signed_integer("1", 32));
        static_cast<void>(mirror::deserialize<std::tuple<std::int32_t, std::int32_t>>(array));
    }, "tuple element count mismatch");

    require_throws_message([] {
        mirror::value variant = mirror::value::object("variant");
        variant.fields.emplace_back("index", mirror::value::unsigned_integer("99", 32));
        variant.fields.emplace_back("value", mirror::value::signed_integer("1", 32));
        static_cast<void>(mirror::deserialize<std::variant<std::int32_t, std::string>>(variant));
    }, "variant index is out of range");

    require_throws_message([] {
        mirror::value variant = mirror::value::object("variant");
        variant.fields.emplace_back("index", mirror::value::unsigned_integer("0", 32));
        static_cast<void>(mirror::deserialize<std::variant<std::int32_t, std::string>>(variant));
    }, "missing required field");

    require_throws_message([] {
        mirror::value map = mirror::value::array();
        map.elements.emplace_back(mirror::value::string("not an entry"));
        static_cast<void>(mirror::deserialize<std::map<std::string, std::int32_t>>(map));
    }, "unexpected value kind");

    require_throws_message([] {
        mirror::value map = mirror::value::array();
        map.elements.emplace_back(mirror::value::object("map_entry"));
        static_cast<void>(mirror::deserialize<std::map<std::string, std::int32_t>>(map));
    }, "missing required field");
}

TEST_CASE("complex nested documents round-trip through JSON and YAML", "[serialization][complex]")
{
    const auto input = make_complex();

    require_equal(input, json_round_trip(input));
    require_equal(input, yaml_round_trip(input));
}
