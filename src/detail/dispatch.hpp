#pragma once

#include "../adapter.hpp"
#include "../adapters/map.hpp"
#include "../adapters/object.hpp"
#include "../adapters/optional.hpp"
#include "../adapters/pointer.hpp"
#include "../adapters/range.hpp"
#include "../adapters/scalar.hpp"
#include "../adapters/tuple.hpp"
#include "../adapters/variant.hpp"

#include <type_traits>

namespace mirror::detail
{

template <typename Type>
struct unsupported_adapter
{
    static mirror::value serialize(const Type&)
    {
        static_assert(always_false<Type>, "type is not serializable by mirror");
    }

    static void deserialize(const mirror::value&, Type&)
    {
        static_assert(always_false<Type>, "type is not deserializable by mirror");
    }
};

template <typename Type>
consteval auto select_adapter()
{
    using ReflectedType = clean_t<Type>;

    if constexpr(custom_adapter<Type>)
    {
        return std::type_identity<mirror::adapter<ReflectedType>>{};
    }
    else if constexpr(pointer_like<Type>)
    {
        return std::type_identity<pointer_adapter<ReflectedType>>{};
    }
    else if constexpr(optional_like<Type>)
    {
        return std::type_identity<optional_adapter<ReflectedType>>{};
    }
    else if constexpr(scalar_like<Type>)
    {
        return std::type_identity<scalar_adapter<ReflectedType>>{};
    }
    else if constexpr(variant_like<Type>)
    {
        return std::type_identity<variant_adapter<ReflectedType>>{};
    }
    else if constexpr(std_array_like<Type> || c_array_like<Type>)
    {
        return std::type_identity<fixed_array_adapter<ReflectedType>>{};
    }
    else if constexpr(map_like<Type>)
    {
        return std::type_identity<map_adapter<ReflectedType>>{};
    }
    else if constexpr(sequence_like<Type>)
    {
        return std::type_identity<sequence_adapter<ReflectedType>>{};
    }
    else if constexpr(tuple_like<Type>)
    {
        return std::type_identity<tuple_adapter<ReflectedType>>{};
    }
    else if constexpr(reflected_object<Type>)
    {
        return std::type_identity<reflection_adapter<ReflectedType>>{};
    }
    else
    {
        return std::type_identity<unsupported_adapter<ReflectedType>>{};
    }
}

template <typename Type>
using selected_adapter_t = typename decltype(select_adapter<Type>())::type;

template <typename Type>
mirror::value serialize_value(const Type& input)
{
    return selected_adapter_t<Type>::serialize(input);
}

template <typename Type>
void deserialize_value(const mirror::value& input, Type& output)
{
    selected_adapter_t<Type>::deserialize(input, output);
}

} // namespace mirror::detail
