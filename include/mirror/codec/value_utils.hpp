#pragma once

#include <mirror/codec/value.hpp>

#include <stdexcept>
#include <string_view>

namespace mirror::codec
{

inline void require_kind(const mirror::value& input, mirror::value::kind expected)
{
    if(input.type != expected)
    {
        throw std::runtime_error{"unexpected value kind"};
    }
}

inline const mirror::value& require_field(const mirror::value& input, std::string_view name)
{
    const auto* field = input.find(name);
    if(field == nullptr)
    {
        throw std::runtime_error{"missing required field"};
    }
    return *field;
}

} // namespace mirror::codec
