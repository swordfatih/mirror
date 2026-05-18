#include <catch2/catch_test_macros.hpp>

#include "support/Fixtures.hpp"

#include <cstdint>
#include <variant>

TEST_CASE("scalar primitives preserve values through JSON and YAML", "[serialization][scalars]")
{
    const auto input = mirror::test::make_scalars();

    mirror::test::require_equal(input, mirror::test::json_round_trip(input));
    mirror::test::require_equal(input, mirror::test::yaml_round_trip(input));
}

TEST_CASE("containers and tuple-like types round-trip", "[serialization][containers]")
{
    const auto input = mirror::test::make_containers();

    mirror::test::require_equal(input, mirror::test::json_round_trip(input));
    mirror::test::require_equal(input, mirror::test::yaml_round_trip(input));
}

TEST_CASE("optional and variant values round-trip", "[serialization][optional][variant]")
{
    const auto input = mirror::test::make_optional_variant();

    mirror::test::require_equal(input, mirror::test::json_round_trip(input));
    mirror::test::require_equal(input, mirror::test::yaml_round_trip(input));
}

TEST_CASE("variant duplicate alternatives preserve the active index", "[serialization][variant]")
{
    const std::variant<std::int32_t, std::int32_t> input{std::in_place_index<1>, 42};

    const auto output = mirror::test::json_round_trip(input);

    REQUIRE(output.index() == 1);
    REQUIRE(std::get<1>(output) == 42);
}

TEST_CASE("complex nested documents round-trip through JSON and YAML", "[serialization][complex]")
{
    const auto input = mirror::test::make_complex();

    mirror::test::require_equal(input, mirror::test::json_round_trip(input));
    mirror::test::require_equal(input, mirror::test::yaml_round_trip(input));
}
