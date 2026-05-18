#pragma once

#include "../value.hpp"

#include <string>
#include <string_view>

namespace mirror::yaml
{

std::string write(const mirror::value& input);
mirror::value read(std::string_view input);

} // namespace mirror::yaml
