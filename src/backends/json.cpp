#include <mirror/backends/json.hpp>

#include <cstddef>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

namespace mirror::json
{
namespace
{

nlohmann::json to_backend(const mirror::value& input)
{
    switch(input.type)
    {
        case mirror::value::kind::object:
        {
            auto output = nlohmann::json::object();
            for(const auto& [name, child]: input.fields)
            {
                output[name] = to_backend(child);
            }
            return output;
        }
        case mirror::value::kind::array:
        {
            auto output = nlohmann::json::array();
            for(const auto& element: input.elements)
            {
                output.push_back(to_backend(element));
            }
            return output;
        }
        case mirror::value::kind::string:
            return input.text;
        case mirror::value::kind::character:
            return nlohmann::json::parse(input.text);
        case mirror::value::kind::signed_integer:
            return nlohmann::json::parse(input.text);
        case mirror::value::kind::unsigned_integer:
            return nlohmann::json::parse(input.text);
        case mirror::value::kind::floating_point:
            return nlohmann::json::parse(input.text);
        case mirror::value::kind::boolean:
            return input.boolean;
        case mirror::value::kind::null:
            return nullptr;
    }

    return nullptr;
}

mirror::value from_backend(const nlohmann::json& input)
{
    if(input.is_object())
    {
        mirror::value output;
        output.type = mirror::value::kind::object;
        for(const auto& [name, child]: input.items())
        {
            output.fields.emplace_back(name, from_backend(child));
        }
        return output;
    }
    if(input.is_array())
    {
        auto output = mirror::value::array();
        for(const auto& element: input)
        {
            output.elements.emplace_back(from_backend(element));
        }
        return output;
    }
    if(input.is_string())
    {
        return mirror::value::string(input.get<std::string>());
    }
    if(input.is_number_unsigned())
    {
        return mirror::value::unsigned_integer(input.dump(), 64);
    }
    if(input.is_number_integer())
    {
        return mirror::value::signed_integer(input.dump(), 64);
    }
    if(input.is_number_float())
    {
        return mirror::value::floating_point(input.dump(), 64);
    }
    if(input.is_boolean())
    {
        return mirror::value::boolean_value(input.get<bool>());
    }

    return {};
}

} // namespace

std::string write(const mirror::value& input)
{
    return to_backend(input).dump();
}

mirror::value read(std::string_view input)
{
    return from_backend(nlohmann::json::parse(input));
}

} // namespace mirror::json
