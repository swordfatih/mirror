#include <catch2/catch_test_macros.hpp>

#include "deserialize.hpp"
#include "serialize.hpp"

#include "support/Fixtures.hpp"

#include <memory>

TEST_CASE("owning smart pointers round-trip and null smart pointers stay null", "[serialization][pointers]")
{
    mirror::test::PointerTypes input;
    input.unique = std::make_unique<mirror::test::Point>(mirror::test::Point{1, 2});
    input.shared = std::make_shared<mirror::test::Point>(mirror::test::Point{3, 4});

    const auto output = mirror::test::json_round_trip(input);

    REQUIRE(output.unique != nullptr);
    REQUIRE(output.shared != nullptr);
    REQUIRE(output.raw == nullptr);
    mirror::test::require_equal(*input.unique, *output.unique);
    mirror::test::require_equal(*input.shared, *output.shared);

    mirror::test::PointerTypes null_input;
    const auto                 null_output = mirror::test::json_round_trip(null_input);
    REQUIRE(null_output.unique == nullptr);
    REQUIRE(null_output.shared == nullptr);
    REQUIRE(null_output.raw == nullptr);
}

TEST_CASE("raw pointers require caller-provided storage during deserialization", "[serialization][pointers]")
{
    mirror::test::Point source_point{5, 6};
    mirror::test::PointerTypes input;
    input.raw = &source_point;

    const auto tree = mirror::serialize(input);

    mirror::test::require_throws_message([&] {
        static_cast<void>(mirror::deserialize<mirror::test::PointerTypes>(tree));
    }, "cannot deserialize into a null raw pointer; use an owning smart pointer or provide valid storage");

    mirror::test::Point storage{};
    mirror::test::PointerTypes output;
    output.raw = &storage;

    mirror::deserialize(tree, output);
    REQUIRE(output.raw == &storage);
    mirror::test::require_equal(source_point, *output.raw);
}
