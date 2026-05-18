#include "value.hpp"

#include <ranges>
#include <stdexcept>

namespace mirror
{

value value::object(std::string type_name)
{
    value result;
    result.type = kind::object;
    result.fields.emplace_back("_mirror_type", value::string(std::move(type_name)));
    return result;
}

value value::string(std::string text)
{
    value result;
    result.type = kind::string;
    result.text = std::move(text);
    return result;
}

value value::character(std::string text, std::size_t bits)
{
    value result;
    result.type = kind::character;
    result.text = std::move(text);
    result.bits = bits;
    return result;
}

value value::signed_integer(std::string text, std::size_t bits)
{
    value result;
    result.type = kind::signed_integer;
    result.text = std::move(text);
    result.bits = bits;
    return result;
}

value value::unsigned_integer(std::string text, std::size_t bits)
{
    value result;
    result.type = kind::unsigned_integer;
    result.text = std::move(text);
    result.bits = bits;
    return result;
}

value value::floating_point(std::string text, std::size_t bits)
{
    value result;
    result.type = kind::floating_point;
    result.text = std::move(text);
    result.bits = bits;
    return result;
}

value value::boolean_value(bool boolean)
{
    value result;
    result.type = kind::boolean;
    result.boolean = boolean;
    return result;
}

value value::array()
{
    value result;
    result.type = kind::array;
    return result;
}

const value* value::find(std::string_view name) const
{
    if(type != kind::object)
    {
        throw std::runtime_error{"expected object"};
    }

    const auto iterator = std::ranges::find_if(fields, [&](const auto& field) {
        return field.first == name;
    });

    return iterator == fields.end() ? nullptr : &iterator->second;
}

} // namespace mirror
