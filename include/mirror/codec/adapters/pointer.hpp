#pragma once

#include <mirror/codec/adapter.hpp>

#include <stdexcept>

namespace mirror::codec
{

template <typename Type>
struct pointer_adapter
{
    static mirror::value serialize(const Type& input)
    {
        if(input == nullptr)
        {
            return {};
        }

        return serialize_value(*input);
    }

    static void deserialize(const mirror::value& input, Type& output)
    {
        if(input.type == mirror::value::kind::null)
        {
            output = nullptr;
            return;
        }

        if constexpr(mirror::codec::resettable_pointer<Type>)
        {
            if(output == nullptr)
            {
                using ElementType = typename mirror::codec::pointer_element<Type>::type;
                output.reset(new ElementType{});
            }
        }
        else if(output == nullptr)
        {
            throw std::runtime_error{
                "cannot deserialize into a null raw pointer; use an owning smart pointer or provide valid storage"
            };
        }

        deserialize_value(input, *output);
    }
};

} // namespace mirror::codec
