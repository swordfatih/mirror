#include <catch2/catch_test_macros.hpp>

#include <mirror/json.hpp>
#include <mirror/mirror.hpp>
#include <mirror/yaml.hpp>

#include "support/Fixtures.hpp"

#include <cstdint>
#include <string>

TEST_CASE("primitive values are emitted as plain JSON scalars", "[serialization][scalars]")
{
    const auto document = mirror::json::write(mirror::serialize(std::int32_t{-5}));

    REQUIRE(document == "-5");
    REQUIRE(document.find("_primitive") == std::string::npos);
    REQUIRE(document.find("_mirror_primitive") == std::string::npos);
    REQUIRE(document.find("_mirror_bits") == std::string::npos);
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
}

TEST_CASE("external YAML numeric syntax is classified as numeric values", "[yaml][external]")
{
    auto positive_integer = mirror::yaml::read("+42");
    REQUIRE(positive_integer.type == mirror::value::kind::signed_integer);
    REQUIRE(positive_integer.text == "42");

    REQUIRE(mirror::yaml::read("1e3").type == mirror::value::kind::floating_point);
    REQUIRE(mirror::yaml::read(".5").type == mirror::value::kind::floating_point);
    REQUIRE(mirror::yaml::read("1.").type == mirror::value::kind::floating_point);
}
