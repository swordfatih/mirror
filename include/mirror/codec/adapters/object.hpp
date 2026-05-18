#pragma once

#include <mirror/codec/adapter.hpp>
#include <mirror/codec/value_utils.hpp>
#include <mirror/meta.hpp>

#include <stdexcept>
#include <string>

namespace mirror::codec
{

template <typename Type>
struct reflection_adapter
{
    using ReflectedType = mirror::codec::clean_t<Type>;

    static mirror::value serialize(const Type& input)
    {
        auto output = mirror::value::object(mirror::meta::type_name<ReflectedType>());

        mirror::meta::for_each_field(input, [&](mirror::meta::field field, const auto& field_value) {
            output.fields.emplace_back(std::string{field.name}, serialize_value(field_value));
        });

        return output;
    }

    static void deserialize(const mirror::value& input, Type& output)
    {
        mirror::codec::require_kind(input, mirror::value::kind::object);

        const auto* type = input.find("_mirror_type");
        if(type != nullptr && type->text != mirror::meta::type_name<ReflectedType>())
        {
            throw std::runtime_error{"unexpected object type"};
        }

        mirror::meta::for_each_field(output, [&](mirror::meta::field field, auto& field_value) {
            deserialize_value(mirror::codec::require_field(input, field.name), field_value);
        });
    }
};

} // namespace mirror::codec
