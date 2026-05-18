#pragma once

#include <mirror/detail/dispatch.hpp>
#include <mirror/value.hpp>

namespace mirror
{

template <typename Type>
void deserialize(const mirror::value& input, Type& output)
{
    mirror::detail::deserialize_value(input, output);
}

template <typename Type>
Type deserialize(const mirror::value& input)
{
    Type output{};
    mirror::deserialize(input, output);
    return output;
}

} // namespace mirror
