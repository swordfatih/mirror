#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace mirror
{

class value
{
public:
    enum class kind
    {
        object,
        array,
        string,
        character,
        signed_integer,
        unsigned_integer,
        floating_point,
        boolean,
        null
    };

    kind                                       type = kind::null;
    std::string                                text;
    bool                                       boolean = false;
    std::size_t                                bits = 0;
    std::vector<std::pair<std::string, value>> fields;
    std::vector<value>                         elements;

    static value object(std::string type_name);
    static value string(std::string text);
    static value character(std::string text, std::size_t bits);
    static value signed_integer(std::string text, std::size_t bits);
    static value unsigned_integer(std::string text, std::size_t bits);
    static value floating_point(std::string text, std::size_t bits);
    static value boolean_value(bool boolean);
    static value array();

    const value* find(std::string_view name) const;
};

} // namespace mirror
