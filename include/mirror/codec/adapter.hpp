#pragma once

#include <mirror/codec/concepts.hpp>
#include <mirror/codec/value.hpp>

#include <concepts>

namespace mirror
{

template <typename Type>
struct adapter;

} // namespace mirror

namespace mirror::codec
{

template <typename Type>
mirror::value serialize_value(const Type& input);

template <typename Type>
void deserialize_value(const mirror::value& input, Type& output);

template <typename Type>
Type deserialize_value(const mirror::value& input)
{
    Type output{};
    deserialize_value(input, output);
    return output;
}

template <typename Type>
concept custom_adapter = requires(
    const mirror::codec::clean_t<Type>& input,
    const mirror::value&                node,
    mirror::codec::clean_t<Type>&       output
) {
    { mirror::adapter<mirror::codec::clean_t<Type>>::serialize(input) } -> std::same_as<mirror::value>;
    mirror::adapter<mirror::codec::clean_t<Type>>::deserialize(node, output);
};

} // namespace mirror::codec
