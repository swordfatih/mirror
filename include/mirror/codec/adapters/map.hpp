#pragma once

#include <mirror/codec/adapter.hpp>
#include <mirror/codec/value_utils.hpp>

#include <utility>

namespace mirror::codec
{

template <typename Type>
struct map_adapter
{
    static mirror::value serialize(const Type& input)
    {
        auto output = mirror::value::array();
        for(const auto& [key, mapped]: input)
        {
            auto entry = mirror::value::object("map_entry");
            entry.fields.emplace_back("key", serialize_value(key));
            entry.fields.emplace_back("value", serialize_value(mapped));
            output.elements.emplace_back(std::move(entry));
        }
        return output;
    }

    static void deserialize(const mirror::value& input, Type& output)
    {
        mirror::codec::require_kind(input, mirror::value::kind::array);
        if constexpr(mirror::codec::clearable<Type>)
        {
            output.clear();
        }

        for(const auto& element: input.elements)
        {
            mirror::codec::require_kind(element, mirror::value::kind::object);
            auto key = deserialize_value<typename mirror::codec::clean_t<Type>::key_type>(
                mirror::codec::require_field(element, "key")
            );
            auto mapped = deserialize_value<typename mirror::codec::clean_t<Type>::mapped_type>(
                mirror::codec::require_field(element, "value")
            );
            output.emplace(std::move(key), std::move(mapped));
        }
    }
};

} // namespace mirror::codec
