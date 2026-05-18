#pragma once

#include <mirror/meta.hpp>

#include <string_view>
#include <type_traits>

namespace mirror::sql
{

template <typename Type>
struct table
{
    static constexpr std::string_view name = mirror::meta::type_name<std::remove_cvref_t<Type>>();
    static constexpr std::string_view primary_key = "id";
};

template <typename Type>
constexpr std::string_view table_name()
{
    return table<std::remove_cvref_t<Type>>::name;
}

template <typename Type>
constexpr std::string_view primary_key()
{
    return table<std::remove_cvref_t<Type>>::primary_key;
}

} // namespace mirror::sql
