#pragma once

#include "../value.hpp"
#include "concepts.hpp"

#include <charconv>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

namespace mirror::detail
{

inline std::string floating_to_string(auto input)
{
    std::ostringstream stream;
    stream << std::setprecision(std::numeric_limits<decltype(input)>::max_digits10) << input;
    return stream.str();
}

inline void require_complete_parse(const char* parsed, const char* end)
{
    if(parsed != end)
    {
        throw std::runtime_error{"invalid numeric value"};
    }
}

template <typename Type>
std::string integer_to_string(Type input)
{
    if constexpr(std::signed_integral<Type>)
    {
        return std::to_string(static_cast<std::intmax_t>(input));
    }
    else
    {
        return std::to_string(static_cast<std::uintmax_t>(input));
    }
}

template <std::signed_integral Type>
Type parse_signed_integer(std::string_view text)
{
    std::intmax_t parsed = 0;
    const auto*   begin = text.data();
    const auto*   end = begin + text.size();
    const auto    result = std::from_chars(begin, end, parsed);

    if(result.ec == std::errc::invalid_argument)
    {
        throw std::runtime_error{"invalid integer value"};
    }
    if(result.ec == std::errc::result_out_of_range)
    {
        throw std::runtime_error{"integer value is out of range for target type"};
    }
    require_complete_parse(result.ptr, end);

    if(parsed < static_cast<std::intmax_t>(std::numeric_limits<Type>::min()) ||
       parsed > static_cast<std::intmax_t>(std::numeric_limits<Type>::max()))
    {
        throw std::runtime_error{"integer value is out of range for target type"};
    }

    return static_cast<Type>(parsed);
}

template <std::unsigned_integral Type>
Type parse_unsigned_integer(std::string_view text)
{
    if(text.starts_with('-'))
    {
        throw std::runtime_error{"integer value is out of range for target type"};
    }

    std::uintmax_t parsed = 0;
    const auto*    begin = text.data();
    const auto*    end = begin + text.size();
    const auto     result = std::from_chars(begin, end, parsed);

    if(result.ec == std::errc::invalid_argument)
    {
        throw std::runtime_error{"invalid integer value"};
    }
    if(result.ec == std::errc::result_out_of_range)
    {
        throw std::runtime_error{"integer value is out of range for target type"};
    }
    require_complete_parse(result.ptr, end);

    if(parsed > static_cast<std::uintmax_t>(std::numeric_limits<Type>::max()))
    {
        throw std::runtime_error{"integer value is out of range for target type"};
    }

    return static_cast<Type>(parsed);
}

template <std::integral Type>
    requires(!std::same_as<Type, bool>)
Type parse_integer(std::string_view text)
{
    if constexpr(std::signed_integral<Type>)
    {
        return parse_signed_integer<Type>(text);
    }
    else
    {
        return parse_unsigned_integer<Type>(text);
    }
}

template <std::floating_point Type>
Type parse_floating_point(std::string_view text)
{
    std::size_t position = 0;
    long double parsed = 0.0L;

    try
    {
        parsed = std::stold(std::string{text}, &position);
    }
    catch(const std::invalid_argument&)
    {
        throw std::runtime_error{"invalid floating-point value"};
    }
    catch(const std::out_of_range&)
    {
        throw std::runtime_error{"floating-point value is out of range for target type"};
    }

    if(position != text.size() || !std::isfinite(parsed))
    {
        throw std::runtime_error{"invalid floating-point value"};
    }

    if(parsed < static_cast<long double>(std::numeric_limits<Type>::lowest()) ||
       parsed > static_cast<long double>(std::numeric_limits<Type>::max()))
    {
        throw std::runtime_error{"floating-point value is out of range for target type"};
    }

    return static_cast<Type>(parsed);
}

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

} // namespace mirror::detail
