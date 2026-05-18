#pragma once

#include <mirror/codec/adapter.hpp>
#include <mirror/codec/value_utils.hpp>

#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace mirror::codec
{

template <typename Range>
mirror::value serialize_range(const Range& input)
{
    auto output = mirror::value::array();
    for(const auto& element: input)
    {
        output.elements.emplace_back(serialize_value(element));
    }
    return output;
}

template <typename Type>
struct fixed_array_adapter
{
    static mirror::value serialize(const Type& input)
    {
        return serialize_range(input);
    }

    static void deserialize(const mirror::value& input, Type& output)
    {
        mirror::codec::require_kind(input, mirror::value::kind::array);

        constexpr bool is_c_array = mirror::codec::c_array_like<Type>;
        const auto     size = [&] {
            if constexpr(is_c_array)
            {
                return std::extent_v<Type>;
            }
            else
            {
                return output.size();
            }
        }();

        if(input.elements.size() != size)
        {
            throw std::runtime_error{"array element count mismatch"};
        }

        for(std::size_t index = 0; index < size; ++index)
        {
            deserialize_value(input.elements[index], output[index]);
        }
    }
};

template <typename Type>
struct sequence_adapter
{
    static mirror::value serialize(const Type& input)
    {
        return serialize_range(input);
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
            auto value = deserialize_value<typename mirror::codec::clean_t<Type>::value_type>(element);
            if constexpr(mirror::codec::push_back_container<Type, decltype(value)>)
            {
                output.push_back(std::move(value));
            }
            else if constexpr(mirror::codec::insert_container<Type, decltype(value)>)
            {
                output.insert(std::move(value));
            }
            else
            {
                static_assert(mirror::codec::always_false<Type>, "sequence container needs push_back or insert");
            }
        }
    }
};

} // namespace mirror::codec
