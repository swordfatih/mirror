#pragma once

#include <mirror/extensions/sql/bind.hpp>

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

namespace mirror::sql
{

template <typename Type>
struct adapter;

template <typename Type>
concept custom_adapter = requires(const Type& value) {
    { mirror::sql::adapter<std::remove_cvref_t<Type>>::bind(value) } -> std::same_as<mirror::sql::bind_value>;
};

template <typename Type>
concept optional_like = requires(Type& value) {
    typename std::remove_cvref_t<Type>::value_type;
    static_cast<bool>(value);
    *value;
};

template <typename Type>
bind_value bind(const Type& value);

template <typename Type>
bind_value bind_optional(const Type& value)
{
    if(!value)
    {
        return nullptr;
    }
    return mirror::sql::bind(*value);
}

template <typename Type>
bind_value bind(const Type& value)
{
    using CleanType = std::remove_cvref_t<Type>;

    if constexpr(custom_adapter<CleanType>)
    {
        return mirror::sql::adapter<CleanType>::bind(value);
    }
    else if constexpr(optional_like<CleanType>)
    {
        return bind_optional(value);
    }
    else if constexpr(std::same_as<CleanType, bool>)
    {
        return value;
    }
    else if constexpr(std::signed_integral<CleanType> && !std::same_as<CleanType, bool>)
    {
        return static_cast<std::int64_t>(value);
    }
    else if constexpr(std::unsigned_integral<CleanType>)
    {
        return static_cast<std::uint64_t>(value);
    }
    else if constexpr(std::floating_point<CleanType>)
    {
        return static_cast<double>(value);
    }
    else if constexpr(std::same_as<CleanType, std::string>)
    {
        return value;
    }
    else if constexpr(std::same_as<CleanType, std::string_view>)
    {
        return std::string{value};
    }
    else if constexpr(std::same_as<CleanType, const char*>)
    {
        return std::string{value};
    }
    else if constexpr(std::same_as<CleanType, std::nullptr_t>)
    {
        return nullptr;
    }
    else
    {
        static_assert(!sizeof(Type), "type cannot be bound to SQL; specialize mirror::sql::adapter<T>");
    }
}

} // namespace mirror::sql
