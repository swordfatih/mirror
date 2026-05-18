#pragma once

#include <mirror/extensions/sql/adapter.hpp>
#include <mirror/extensions/sql/statement.hpp>
#include <mirror/extensions/sql/table.hpp>
#include <mirror/meta.hpp>

#include <string>
#include <string_view>
#include <stdexcept>
#include <utility>

namespace mirror::sql
{
namespace detail
{

inline void append_identifier(std::string& output, std::string_view identifier)
{
    output.append(identifier);
}

inline void append_placeholders(std::string& output, std::size_t count)
{
    for(std::size_t index = 0; index < count; ++index)
    {
        if(index != 0)
        {
            output.append(", ");
        }
        output.push_back('?');
    }
}

template <typename Model>
std::size_t field_count(Model& model)
{
    std::size_t count = 0;
    mirror::meta::for_each_field(model, [&](mirror::meta::field, auto&) {
        ++count;
    });
    return count;
}

} // namespace detail

template <typename Model>
statement insert(const Model& model)
{
    statement output;
    output.text = "INSERT INTO ";
    detail::append_identifier(output.text, table_name<Model>());
    output.text.append(" (");

    bool first = true;
    mirror::meta::for_each_field(model, [&](mirror::meta::field field, const auto& value) {
        if(!first)
        {
            output.text.append(", ");
        }
        first = false;
        detail::append_identifier(output.text, field.name);
        output.binds.emplace_back(mirror::sql::bind(value));
    });

    output.text.append(") VALUES (");
    detail::append_placeholders(output.text, output.binds.size());
    output.text.push_back(')');
    return output;
}

template <typename Model>
statement update(const Model& model)
{
    statement output;
    output.text = "UPDATE ";
    detail::append_identifier(output.text, table_name<Model>());
    output.text.append(" SET ");

    const auto key = primary_key<Model>();
    bind_value  key_bind = nullptr;
    bool        found_key = false;
    bool        first = true;

    mirror::meta::for_each_field(model, [&](mirror::meta::field field, const auto& value) {
        if(field.name == key)
        {
            key_bind = mirror::sql::bind(value);
            found_key = true;
            return;
        }

        if(!first)
        {
            output.text.append(", ");
        }
        first = false;
        detail::append_identifier(output.text, field.name);
        output.text.append(" = ?");
        output.binds.emplace_back(mirror::sql::bind(value));
    });

    output.text.append(" WHERE ");
    detail::append_identifier(output.text, key);
    output.text.append(" = ?");
    output.binds.emplace_back(std::move(key_bind));

    if(!found_key)
    {
        throw std::runtime_error{"primary key field was not found"};
    }

    return output;
}

template <typename Model, typename Id>
statement select_by_id(const Id& id)
{
    statement output;
    output.text = "SELECT ";

    Model model{};
    bool  first = true;
    mirror::meta::for_each_field(model, [&](mirror::meta::field field, auto&) {
        if(!first)
        {
            output.text.append(", ");
        }
        first = false;
        detail::append_identifier(output.text, field.name);
    });

    output.text.append(" FROM ");
    detail::append_identifier(output.text, table_name<Model>());
    output.text.append(" WHERE ");
    detail::append_identifier(output.text, primary_key<Model>());
    output.text.append(" = ?");
    output.binds.emplace_back(mirror::sql::bind(id));
    return output;
}

template <typename Model, typename Id>
statement delete_by_id(const Id& id)
{
    statement output;
    output.text = "DELETE FROM ";
    detail::append_identifier(output.text, table_name<Model>());
    output.text.append(" WHERE ");
    detail::append_identifier(output.text, primary_key<Model>());
    output.text.append(" = ?");
    output.binds.emplace_back(mirror::sql::bind(id));
    return output;
}

} // namespace mirror::sql
