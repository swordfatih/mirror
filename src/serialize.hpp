#pragma once

#include "detail/dispatch.hpp"
#include "value.hpp"

namespace mirror
{

template <typename Type>
mirror::value serialize(const Type& input)
{
    return mirror::detail::serialize_value(input);
}

} // namespace mirror
