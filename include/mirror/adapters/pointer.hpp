#pragma once

#include <mirror/adapter.hpp>

#include <stdexcept>

namespace mirror::detail
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

        if constexpr(resettable_pointer<Type>)
        {
            if(output == nullptr)
            {
                using ElementType = typename pointer_element<Type>::type;
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

} // namespace mirror::detail
