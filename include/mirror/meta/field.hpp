#pragma once

#include <mirror/meta/reflect.hpp>

#include <string_view>
#include <type_traits>
#include <utility>

namespace mirror::meta
{

struct field
{
    std::string_view name;
};

template <typename Type>
consteval auto type_name()
{
    return mirror::meta::reflect<std::remove_cvref_t<Type>>::name();
}

template <typename Object, typename Function>
constexpr void for_each_field(Object&& object, Function&& function)
{
    using ReflectedType = std::remove_cvref_t<Object>;

    mirror::meta::reflect<ReflectedType>::for_each_field(
        std::forward<Object>(object),
        [&](std::string_view name, auto& value) {
            function(mirror::meta::field{name}, value);
        }
    );
}

} // namespace mirror::meta
