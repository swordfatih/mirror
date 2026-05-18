#pragma once

#include "../adapter.hpp"

#include "../detail/utils.hpp"
#include "../reflect.hpp"

#include <stdexcept>
#include <string>
#include <string_view>

namespace mirror::detail
{

template <typename Type>
struct reflection_adapter
{
    using ReflectedType = clean_t<Type>;

    static mirror::value serialize(const Type& input)
    {
        auto output = mirror::value::object(mirror::reflect<ReflectedType>::name());

        mirror::reflect<ReflectedType>::for_each_field(input, [&](std::string_view name, const auto& field_value) {
            output.fields.emplace_back(std::string{name}, serialize_value(field_value));
        });

        return output;
    }

    static void deserialize(const mirror::value& input, Type& output)
    {
        require_kind(input, mirror::value::kind::object);

        const auto* type = input.find("_mirror_type");
        if(type != nullptr && type->text != mirror::reflect<ReflectedType>::name())
        {
            throw std::runtime_error{"unexpected object type"};
        }

        mirror::reflect<ReflectedType>::for_each_field(output, [&](std::string_view name, auto& field_value) {
            deserialize_value(require_field(input, name), field_value);
        });
    }
};

} // namespace mirror::detail
