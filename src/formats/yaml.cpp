#include <mirror/formats/yaml.hpp>

#include <cctype>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <yaml-cpp/yaml.h>

namespace mirror::yaml
{
namespace
{

enum class number_kind
{
    none,
    integer,
    floating_point
};

YAML::Node to_format(const mirror::value& input)
{
    YAML::Node output;

    switch(input.type)
    {
        case mirror::value::kind::object:
            output = YAML::Node{YAML::NodeType::Map};
            for(const auto& [name, child]: input.fields)
            {
                output[name] = to_format(child);
            }
            break;
        case mirror::value::kind::array:
            output = YAML::Node{YAML::NodeType::Sequence};
            for(const auto& element: input.elements)
            {
                output.push_back(to_format(element));
            }
            break;
        case mirror::value::kind::string:
            output = input.text;
            break;
        case mirror::value::kind::character:
            output = YAML::Load(input.text);
            break;
        case mirror::value::kind::signed_integer:
            output = YAML::Load(input.text);
            break;
        case mirror::value::kind::unsigned_integer:
            output = YAML::Load(input.text);
            break;
        case mirror::value::kind::floating_point:
            output = YAML::Load(input.text);
            break;
        case mirror::value::kind::boolean:
            output = input.boolean;
            break;
        case mirror::value::kind::null:
            output = YAML::Node{};
            break;
    }

    return output;
}

number_kind classify_number(std::string_view input)
{
    if(input.empty())
    {
        return number_kind::none;
    }

    std::size_t position = 0;
    if(input[position] == '-' || input[position] == '+')
    {
        ++position;
    }
    if(position == input.size())
    {
        return number_kind::none;
    }

    bool saw_digit_before_dot = false;
    while(position < input.size() && std::isdigit(static_cast<unsigned char>(input[position])))
    {
        saw_digit_before_dot = true;
        ++position;
    }

    bool saw_decimal = false;
    bool saw_digit_after_dot = false;
    if(position < input.size() && input[position] == '.')
    {
        saw_decimal = true;
        ++position;
        while(position < input.size() && std::isdigit(static_cast<unsigned char>(input[position])))
        {
            saw_digit_after_dot = true;
            ++position;
        }
    }

    if(!saw_digit_before_dot && !saw_digit_after_dot)
    {
        return number_kind::none;
    }

    bool saw_exponent = false;
    if(position < input.size() && (input[position] == 'e' || input[position] == 'E'))
    {
        saw_exponent = true;
        ++position;
        if(position < input.size() && (input[position] == '-' || input[position] == '+'))
        {
            ++position;
        }

        bool saw_exponent_digit = false;
        while(position < input.size() && std::isdigit(static_cast<unsigned char>(input[position])))
        {
            saw_exponent_digit = true;
            ++position;
        }
        if(!saw_exponent_digit)
        {
            return number_kind::none;
        }
    }

    if(position != input.size())
    {
        return number_kind::none;
    }

    return saw_decimal || saw_exponent ? number_kind::floating_point : number_kind::integer;
}

std::string normalize_number(std::string_view input)
{
    if(input.starts_with('+'))
    {
        input.remove_prefix(1);
    }
    return std::string{input};
}

mirror::value from_format(const YAML::Node& input)
{
    if(!input || input.IsNull())
    {
        return {};
    }
    if(input.IsMap())
    {
        mirror::value output;
        output.type = mirror::value::kind::object;
        for(const auto& field: input)
        {
            output.fields.emplace_back(field.first.as<std::string>(), from_format(field.second));
        }
        return output;
    }
    if(input.IsSequence())
    {
        auto output = mirror::value::array();
        for(const auto& element: input)
        {
            output.elements.emplace_back(from_format(element));
        }
        return output;
    }
    if(input.IsScalar())
    {
        const auto scalar = input.as<std::string>();
        if(scalar == "true" || scalar == "false")
        {
            return mirror::value::boolean_value(scalar == "true");
        }
        const auto numeric_kind = classify_number(scalar);
        if(numeric_kind == number_kind::floating_point)
        {
            return mirror::value::floating_point(normalize_number(scalar), 64);
        }
        if(numeric_kind == number_kind::integer)
        {
            return mirror::value::signed_integer(normalize_number(scalar), 64);
        }
        return mirror::value::string(scalar);
    }

    throw std::runtime_error{"unsupported YAML node"};
}

} // namespace

std::string write(const mirror::value& input)
{
    YAML::Emitter emitter;
    emitter << to_format(input);
    return emitter.c_str();
}

mirror::value read(std::string_view input)
{
    return from_format(YAML::Load(std::string{input}));
}

} // namespace mirror::yaml
