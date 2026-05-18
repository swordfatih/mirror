#pragma once

#include <mirror/adapter.hpp>

namespace mirror::detail
{

template <typename Type>
struct optional_adapter
{
    static mirror::value serialize(const Type& input)
    {
        if(!input)
        {
            return {};
        }

        return serialize_value(*input);
    }

    static void deserialize(const mirror::value& input, Type& output)
    {
        if(input.type == mirror::value::kind::null)
        {
            output.reset();
            return;
        }

        output.emplace();
        deserialize_value(input, *output);
    }
};

} // namespace mirror::detail
