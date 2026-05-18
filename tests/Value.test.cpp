#include <catch2/catch_test_macros.hpp>

#include <mirror/mirror.hpp>

#include <stdexcept>

TEST_CASE("mirror value factories and lookup behave predictably", "[value]")
{
    auto object = mirror::value::object("Point");
    object.fields.emplace_back("x", mirror::value::signed_integer("10", 32));

    REQUIRE(object.type == mirror::value::kind::object);
    REQUIRE(object.find("_mirror_type") != nullptr);
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
