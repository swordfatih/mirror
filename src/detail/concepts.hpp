#pragma once

#include "../reflect.hpp"

#include <array>
#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace mirror::detail
{

template <typename>
inline constexpr bool always_false = false;

template <typename Type>
using clean_t = std::remove_cvref_t<Type>;

template <typename Type, typename = void>
struct pointer_element
{
    using type = std::remove_pointer_t<clean_t<Type>>;
};

template <typename Type>
struct pointer_element<Type, std::void_t<typename clean_t<Type>::element_type>>
{
    using type = typename clean_t<Type>::element_type;
};

template <typename Type>
concept pointer_like = !std::is_bounded_array_v<clean_t<Type>> && requires(Type& value) {
    value == nullptr;
    *value;
};

template <typename Type>
concept resettable_pointer = requires(Type& value) {
    typename pointer_element<Type>::type;
    value.reset(static_cast<typename pointer_element<Type>::type*>(nullptr));
};

template <typename Type>
concept optional_like = requires(Type& value) {
    typename clean_t<Type>::value_type;
    static_cast<bool>(value);
    *value;
    value.reset();
    value.emplace();
};

template <typename Type>
concept string_like = std::same_as<clean_t<Type>, std::string>;

template <typename Type>
concept char_like = std::same_as<clean_t<Type>, char> ||
                    std::same_as<clean_t<Type>, wchar_t> ||
                    std::same_as<clean_t<Type>, char8_t> ||
                    std::same_as<clean_t<Type>, char16_t> ||
                    std::same_as<clean_t<Type>, char32_t>;

template <typename Type>
concept byte_like = std::same_as<clean_t<Type>, std::byte>;

template <typename Type>
struct is_variant : std::false_type
{
};

template <typename... Types>
struct is_variant<std::variant<Types...>> : std::true_type
{
};

template <typename Type>
concept variant_like = is_variant<clean_t<Type>>::value;

template <typename Type>
struct is_std_array : std::false_type
{
};

template <typename Type, std::size_t Size>
struct is_std_array<std::array<Type, Size>> : std::true_type
{
};

template <typename Type>
concept std_array_like = is_std_array<clean_t<Type>>::value;

template <typename Type>
concept c_array_like = std::is_bounded_array_v<clean_t<Type>>;

template <typename Type>
concept map_like = requires(Type& value) {
    typename clean_t<Type>::key_type;
    typename clean_t<Type>::mapped_type;
} && std::ranges::input_range<clean_t<Type>>;

template <typename Type>
concept sequence_like = !string_like<Type> && !map_like<Type> && requires(Type& value) {
    typename clean_t<Type>::value_type;
} && std::ranges::input_range<clean_t<Type>>;

template <typename Type>
concept clearable = requires(Type& value) {
    value.clear();
};

template <typename Type, typename Value>
concept push_back_container = requires(Type& output, Value&& value) {
    output.push_back(std::forward<Value>(value));
};

template <typename Type, typename Value>
concept insert_container = requires(Type& output, Value&& value) {
    output.insert(std::forward<Value>(value));
};

template <typename Type>
concept tuple_like = !std_array_like<Type> && requires {
    typename std::tuple_size<clean_t<Type>>::type;
};

template <typename Type>
concept reflected_object = requires(Type& value) {
    mirror::reflect<clean_t<Type>>::name();
    mirror::reflect<clean_t<Type>>::for_each_field(value, [](std::string_view, auto&) {});
};

} // namespace mirror::detail
