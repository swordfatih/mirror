#pragma once

#include <mirror/codec/dispatch.hpp>
#include <mirror/codec/value.hpp>

namespace mirror::codec
{

template <typename Type>
mirror::value serialize(const Type& input)
{
    return mirror::codec::serialize_value(input);
}

} // namespace mirror::codec

namespace mirror
{

template <typename Type>
mirror::value serialize(const Type& input)
{
    return mirror::codec::serialize(input);
}

} // namespace mirror
