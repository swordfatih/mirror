#include <catch2/catch_test_macros.hpp>

#include "adapter.hpp"
#include "deserialize.hpp"
#include "serialize.hpp"
#include "value.hpp"

#include <string>

namespace mirror::test
{

struct CustomId
{
    int value = 0;
};

} // namespace mirror::test

namespace mirror
{

template <>
struct adapter<mirror::test::CustomId>
{
    static mirror::value serialize(const mirror::test::CustomId& input)
    {
        return mirror::value::string(std::to_string(input.value));
    }

    static void deserialize(const mirror::value& input, mirror::test::CustomId& output)
    {
        output.value = std::stoi(input.text);
    }
};

} // namespace mirror

TEST_CASE("custom adapters override built-in dispatch", "[adapter]")
{
    const mirror::test::CustomId input{42};

    const auto tree = mirror::serialize(input);
    REQUIRE(tree.type == mirror::value::kind::string);
    REQUIRE(tree.text == "42");

    const auto output = mirror::deserialize<mirror::test::CustomId>(tree);
    REQUIRE(output.value == 42);
}
