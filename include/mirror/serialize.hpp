#pragma once

#include <mirror/detail/dispatch.hpp>
#include <mirror/value.hpp>

namespace mirror
{

template <typename Type>
mirror::value serialize(const Type& input)
{
    return mirror::detail::serialize_value(input);
}

} // namespace mirror
