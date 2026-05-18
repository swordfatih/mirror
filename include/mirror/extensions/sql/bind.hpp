#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace mirror::sql
{

using blob = std::vector<unsigned char>;

using bind_value = std::variant<
    std::nullptr_t,
    bool,
    std::int64_t,
    std::uint64_t,
    double,
    std::string,
    blob
>;

} // namespace mirror::sql
