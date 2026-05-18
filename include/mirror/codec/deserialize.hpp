#pragma once

#include <mirror/codec/dispatch.hpp>
#include <mirror/codec/value.hpp>

namespace mirror::codec
{

template <typename Type>
void deserialize(const mirror::value& input, Type& output)
{
    mirror::codec::deserialize_value(input, output);
}

template <typename Type>
Type deserialize(const mirror::value& input)
{
    Type output{};
    mirror::codec::deserialize(input, output);
    return output;
}

} // namespace mirror::codec

namespace mirror
{

template <typename Type>
void deserialize(const mirror::value& input, Type& output)
{
    mirror::codec::deserialize(input, output);
}

template <typename Type>
Type deserialize(const mirror::value& input)
{
    return mirror::codec::deserialize<Type>(input);
}

} // namespace mirror
