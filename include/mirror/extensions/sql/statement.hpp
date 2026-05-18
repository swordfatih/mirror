#pragma once

#include <mirror/extensions/sql/bind.hpp>

#include <string>
#include <vector>

namespace mirror::sql
{

struct statement
{
    std::string             text;
    std::vector<bind_value> binds;
};

} // namespace mirror::sql
