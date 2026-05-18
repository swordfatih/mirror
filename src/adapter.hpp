#pragma once

#include "detail/concepts.hpp"
#include "value.hpp"

#include <concepts>

namespace mirror
{

template <typename Type>
struct adapter;

} // namespace mirror

namespace mirror::detail
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
concept custom_adapter = requires(const clean_t<Type>& input, const mirror::value& node, clean_t<Type>& output) {
    { mirror::adapter<clean_t<Type>>::serialize(input) } -> std::same_as<mirror::value>;
    mirror::adapter<clean_t<Type>>::deserialize(node, output);
};

} // namespace mirror::detail
