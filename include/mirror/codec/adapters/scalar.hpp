#pragma once

#include <mirror/codec/adapter.hpp>
#include <mirror/codec/scalar_utils.hpp>
#include <mirror/codec/value_utils.hpp>

#include <bit>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>

namespace mirror::codec
{

template <typename Type>
concept scalar_like = std::same_as<mirror::codec::clean_t<Type>, std::nullptr_t> ||
                      std::same_as<mirror::codec::clean_t<Type>, bool> ||
                      std::same_as<mirror::codec::clean_t<Type>, std::monostate> ||
                      mirror::codec::char_like<Type> ||
                      mirror::codec::byte_like<Type> ||
                      std::is_enum_v<mirror::codec::clean_t<Type>> ||
                      (std::integral<mirror::codec::clean_t<Type>> &&
                       !std::same_as<mirror::codec::clean_t<Type>, bool>) ||
                      std::floating_point<mirror::codec::clean_t<Type>> ||
                      mirror::codec::string_like<Type>;

template <typename Type>
struct scalar_adapter
{
    using ReflectedType = mirror::codec::clean_t<Type>;

    static mirror::value serialize(const Type& input)
    {
        if constexpr(std::same_as<ReflectedType, std::nullptr_t>)
        {
            return {};
        }
        else if constexpr(std::same_as<ReflectedType, bool>)
        {
            return mirror::value::boolean_value(input);
        }
        else if constexpr(std::same_as<ReflectedType, std::monostate>)
        {
            return {};
        }
        else if constexpr(mirror::codec::char_like<Type>)
        {
            using CodeUnit = std::conditional_t<std::signed_integral<ReflectedType>, std::intmax_t, std::uintmax_t>;
            return mirror::value::character(std::to_string(static_cast<CodeUnit>(input)), sizeof(ReflectedType) * 8);
        }
        else if constexpr(mirror::codec::byte_like<Type>)
        {
            return mirror::value::unsigned_integer(std::to_string(std::to_integer<unsigned int>(input)), 8);
        }
        else if constexpr(std::is_enum_v<ReflectedType>)
        {
            using UnderlyingType = std::underlying_type_t<ReflectedType>;
            return serialize_value(static_cast<UnderlyingType>(input));
        }
        else if constexpr(std::signed_integral<ReflectedType>)
        {
            return mirror::value::signed_integer(mirror::codec::integer_to_string(input), sizeof(ReflectedType) * 8);
        }
        else if constexpr(std::unsigned_integral<ReflectedType>)
        {
            return mirror::value::unsigned_integer(mirror::codec::integer_to_string(input), sizeof(ReflectedType) * 8);
        }
        else if constexpr(std::floating_point<ReflectedType>)
        {
            return mirror::value::floating_point(mirror::codec::floating_to_string(input), sizeof(ReflectedType) * 8);
        }
        else if constexpr(mirror::codec::string_like<Type>)
        {
            return mirror::value::string(input);
        }
    }

    static void deserialize(const mirror::value& input, Type& output)
    {
        if constexpr(std::same_as<ReflectedType, std::nullptr_t>)
        {
            mirror::codec::require_kind(input, mirror::value::kind::null);
            output = nullptr;
        }
        else if constexpr(std::same_as<ReflectedType, bool>)
        {
            mirror::codec::require_kind(input, mirror::value::kind::boolean);
            output = input.boolean;
        }
        else if constexpr(std::same_as<ReflectedType, std::monostate>)
        {
            mirror::codec::require_kind(input, mirror::value::kind::null);
            output = std::monostate{};
        }
        else if constexpr(mirror::codec::char_like<Type>)
        {
            if(input.type != mirror::value::kind::character &&
               input.type != mirror::value::kind::signed_integer &&
               input.type != mirror::value::kind::unsigned_integer)
            {
                throw std::runtime_error{"unexpected value kind"};
            }
            output = mirror::codec::parse_integer<ReflectedType>(input.text);
        }
        else if constexpr(mirror::codec::byte_like<Type>)
        {
            if(input.type != mirror::value::kind::unsigned_integer && input.type != mirror::value::kind::signed_integer)
            {
                throw std::runtime_error{"unexpected value kind"};
            }
            output = static_cast<std::byte>(mirror::codec::parse_integer<std::uint8_t>(input.text));
        }
        else if constexpr(std::is_enum_v<ReflectedType>)
        {
            using UnderlyingType = std::underlying_type_t<ReflectedType>;
            auto underlying = deserialize_value<UnderlyingType>(input);
            output = static_cast<ReflectedType>(underlying);
        }
        else if constexpr(std::signed_integral<ReflectedType>)
        {
            if(input.type == mirror::value::kind::signed_integer || input.type == mirror::value::kind::unsigned_integer)
            {
                output = mirror::codec::parse_integer<ReflectedType>(input.text);
            }
            else
            {
                throw std::runtime_error{"expected integer"};
            }
        }
        else if constexpr(std::unsigned_integral<ReflectedType>)
        {
            if(input.type == mirror::value::kind::unsigned_integer || input.type == mirror::value::kind::signed_integer)
            {
                output = mirror::codec::parse_integer<ReflectedType>(input.text);
            }
            else
            {
                throw std::runtime_error{"expected integer"};
            }
        }
        else if constexpr(std::floating_point<ReflectedType>)
        {
            if(input.type != mirror::value::kind::floating_point &&
               input.type != mirror::value::kind::signed_integer &&
               input.type != mirror::value::kind::unsigned_integer)
            {
                throw std::runtime_error{"unexpected value kind"};
            }
            output = mirror::codec::parse_floating_point<ReflectedType>(input.text);
        }
        else if constexpr(mirror::codec::string_like<Type>)
        {
            mirror::codec::require_kind(input, mirror::value::kind::string);
            output = input.text;
        }
    }
};

} // namespace mirror::codec
