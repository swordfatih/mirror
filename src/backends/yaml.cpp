#include "backends/yaml.hpp"

#include <cctype>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <yaml-cpp/yaml.h>

namespace mirror::yaml
{
namespace
{

YAML::Node to_backend(const mirror::value& input)
{
    YAML::Node output;

    switch(input.type)
    {
        case mirror::value::kind::object:
            output = YAML::Node{YAML::NodeType::Map};
            for(const auto& [name, child]: input.fields)
            {
                output[name] = to_backend(child);
            }
            break;
        case mirror::value::kind::array:
            output = YAML::Node{YAML::NodeType::Sequence};
            for(const auto& element: input.elements)
            {
                output.push_back(to_backend(element));
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

bool is_number(std::string_view input)
{
    if(input.empty())
    {
        return false;
    }

    std::size_t position = 0;
    if(input[position] == '-')
    {
        ++position;
    }

    bool saw_digit = false;
    while(position < input.size() && std::isdigit(static_cast<unsigned char>(input[position])))
    {
        saw_digit = true;
        ++position;
    }

    if(position < input.size() && input[position] == '.')
    {
        ++position;
        while(position < input.size() && std::isdigit(static_cast<unsigned char>(input[position])))
        {
            saw_digit = true;
            ++position;
        }
    }

    return saw_digit && position == input.size();
}

mirror::value from_backend(const YAML::Node& input)
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
            output.fields.emplace_back(field.first.as<std::string>(), from_backend(field.second));
        }
        return output;
    }
    if(input.IsSequence())
    {
        auto output = mirror::value::array();
        for(const auto& element: input)
        {
            output.elements.emplace_back(from_backend(element));
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
        if(is_number(scalar))
        {
            if(scalar.find('.') != std::string::npos)
            {
                return mirror::value::floating_point(scalar, 64);
            }
            return mirror::value::signed_integer(scalar, 64);
        }
        return mirror::value::string(scalar);
    }

    throw std::runtime_error{"unsupported YAML node"};
}

} // namespace

std::string write(const mirror::value& input)
{
    YAML::Emitter emitter;
    emitter << to_backend(input);
    return emitter.c_str();
}

mirror::value read(std::string_view input)
{
    return from_backend(YAML::Load(std::string{input}));
}

} // namespace mirror::yaml
