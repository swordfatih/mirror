#pragma once

#include <mirror/codec/adapter.hpp>
#include <mirror/codec/value_utils.hpp>

#include <cstddef>
#include <stdexcept>
#include <tuple>

namespace mirror::codec
{

template <typename Tuple, std::size_t... Indexes>
mirror::value serialize_tuple(const Tuple& input, std::index_sequence<Indexes...>)
{
    auto output = mirror::value::array();
    (output.elements.emplace_back(serialize_value(std::get<Indexes>(input))), ...);
    return output;
}

template <typename Tuple, std::size_t... Indexes>
void deserialize_tuple(const mirror::value& input, Tuple& output, std::index_sequence<Indexes...>)
{
    mirror::codec::require_kind(input, mirror::value::kind::array);
    if(input.elements.size() != sizeof...(Indexes))
    {
        throw std::runtime_error{"tuple element count mismatch"};
    }

    (deserialize_value(input.elements[Indexes], std::get<Indexes>(output)), ...);
}

template <typename Type>
struct tuple_adapter
{
    static mirror::value serialize(const Type& input)
    {
        return serialize_tuple(input, std::make_index_sequence<std::tuple_size_v<mirror::codec::clean_t<Type>>>{});
    }

    static void deserialize(const mirror::value& input, Type& output)
    {
        deserialize_tuple(input, output, std::make_index_sequence<std::tuple_size_v<mirror::codec::clean_t<Type>>>{});
    }
};

} // namespace mirror::codec
