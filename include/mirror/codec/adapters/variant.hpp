#pragma once

#include <mirror/codec/adapter.hpp>
#include <mirror/codec/value_utils.hpp>

#include <cstddef>
#include <stdexcept>
#include <string>
#include <variant>

namespace mirror::codec
{

template <typename Variant, std::size_t Index = 0>
void deserialize_variant_alternative(std::size_t active_index, const mirror::value& input, Variant& output)
{
    if constexpr(Index < std::variant_size_v<mirror::codec::clean_t<Variant>>)
    {
        if(active_index == Index)
        {
            using Alternative = std::variant_alternative_t<Index, mirror::codec::clean_t<Variant>>;
            output.template emplace<Index>(deserialize_value<Alternative>(input));
            return;
        }

        deserialize_variant_alternative<Variant, Index + 1>(active_index, input, output);
    }
    else
    {
        throw std::runtime_error{"variant index is out of range"};
    }
}

template <typename Type>
struct variant_adapter
{
    static mirror::value serialize(const Type& input)
    {
        auto output = mirror::value::object("variant");
        output.fields.emplace_back("index", mirror::value::unsigned_integer(std::to_string(input.index()), 32));
        std::visit([&](const auto& value) {
            output.fields.emplace_back("value", serialize_value(value));
        }, input);
        return output;
    }

    static void deserialize(const mirror::value& input, Type& output)
    {
        mirror::codec::require_kind(input, mirror::value::kind::object);
        const auto index = deserialize_value<std::size_t>(mirror::codec::require_field(input, "index"));
        deserialize_variant_alternative(index, mirror::codec::require_field(input, "value"), output);
    }
};

} // namespace mirror::codec
