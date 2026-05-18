#include <catch2/catch_test_macros.hpp>

#include <mirror/mirror.hpp>

#include "support/Fixtures.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <tuple>
#include <variant>

TEST_CASE("deserialization reports scalar kind mismatches", "[serialization][errors]")
{
    mirror::test::require_throws_message([] {
        static_cast<void>(mirror::deserialize<bool>(mirror::value::string("true")));
    }, "unexpected value kind");

    mirror::test::require_throws_message([] {
        static_cast<void>(mirror::deserialize<std::int32_t>(mirror::value::string("1")));
    }, "expected integer");

    mirror::test::require_throws_message([] {
        static_cast<void>(mirror::deserialize<std::uint32_t>(mirror::value::string("1")));
    }, "expected integer");

    mirror::test::require_throws_message([] {
        static_cast<void>(mirror::deserialize<std::string>(mirror::value::signed_integer("1", 32)));
    }, "unexpected value kind");

    mirror::test::require_throws_message([] {
        static_cast<void>(mirror::deserialize<char>(mirror::value::string("a")));
    }, "unexpected value kind");

    mirror::test::require_throws_message([] {
        static_cast<void>(mirror::deserialize<std::byte>(mirror::value::string("1")));
    }, "unexpected value kind");

    mirror::test::require_throws_message([] {
        static_cast<void>(mirror::deserialize<std::nullptr_t>(mirror::value::string("null")));
    }, "unexpected value kind");

    mirror::test::require_throws_message([] {
        static_cast<void>(mirror::deserialize<std::monostate>(mirror::value::string("null")));
    }, "unexpected value kind");
}

TEST_CASE("deserialization reports numeric conversion failures", "[serialization][errors]")
{
    mirror::test::require_throws_message([] {
        static_cast<void>(mirror::deserialize<std::int8_t>(mirror::value::signed_integer("128", 64)));
    }, "integer value is out of range for target type");

    mirror::test::require_throws_message([] {
        static_cast<void>(mirror::deserialize<std::uint8_t>(mirror::value::signed_integer("-1", 64)));
    }, "integer value is out of range for target type");

    mirror::test::require_throws_message([] {
        static_cast<void>(mirror::deserialize<std::byte>(mirror::value::unsigned_integer("256", 64)));
    }, "integer value is out of range for target type");

    mirror::test::require_throws_message([] {
        static_cast<void>(mirror::deserialize<std::int32_t>(mirror::value::signed_integer("12x", 64)));
    }, "invalid numeric value");

    mirror::test::require_throws_message([] {
        static_cast<void>(mirror::deserialize<float>(mirror::value::floating_point("1e100", 64)));
    }, "floating-point value is out of range for target type");
}

TEST_CASE("deserialization reports structural mismatches", "[serialization][errors]")
{
    mirror::test::require_throws_message([] {
        auto object = mirror::value::object("OtherPoint");
        object.fields.emplace_back("x", mirror::value::signed_integer("1", 32));
        object.fields.emplace_back("y", mirror::value::signed_integer("2", 32));
        static_cast<void>(mirror::deserialize<mirror::test::Point>(object));
    }, "unexpected object type");

    mirror::test::require_throws_message([] {
        auto object = mirror::value::object("Point");
        object.fields.emplace_back("x", mirror::value::signed_integer("1", 32));
        static_cast<void>(mirror::deserialize<mirror::test::Point>(object));
    }, "missing required field");

    mirror::test::require_throws_message([] {
        static_cast<void>(mirror::deserialize<std::array<std::int32_t, 2>>(mirror::value::array()));
    }, "array element count mismatch");

    mirror::test::require_throws_message([] {
        mirror::value array = mirror::value::array();
        array.elements.emplace_back(mirror::value::signed_integer("1", 32));
        std::int32_t output[2]{};
        mirror::deserialize(array, output);
    }, "array element count mismatch");

    mirror::test::require_throws_message([] {
        mirror::value array = mirror::value::array();
        array.elements.emplace_back(mirror::value::signed_integer("1", 32));
        static_cast<void>(mirror::deserialize<std::tuple<std::int32_t, std::int32_t>>(array));
    }, "tuple element count mismatch");

    mirror::test::require_throws_message([] {
        mirror::value variant = mirror::value::object("variant");
        variant.fields.emplace_back("index", mirror::value::unsigned_integer("99", 32));
        variant.fields.emplace_back("value", mirror::value::signed_integer("1", 32));
        static_cast<void>(mirror::deserialize<std::variant<std::int32_t, std::string>>(variant));
    }, "variant index is out of range");

    mirror::test::require_throws_message([] {
        mirror::value variant = mirror::value::object("variant");
        variant.fields.emplace_back("index", mirror::value::unsigned_integer("0", 32));
        static_cast<void>(mirror::deserialize<std::variant<std::int32_t, std::string>>(variant));
    }, "missing required field");

    mirror::test::require_throws_message([] {
        mirror::value map = mirror::value::array();
        map.elements.emplace_back(mirror::value::string("not an entry"));
        static_cast<void>(mirror::deserialize<std::map<std::string, std::int32_t>>(map));
    }, "unexpected value kind");

    mirror::test::require_throws_message([] {
        mirror::value map = mirror::value::array();
        map.elements.emplace_back(mirror::value::object("map_entry"));
        static_cast<void>(mirror::deserialize<std::map<std::string, std::int32_t>>(map));
    }, "missing required field");
}
