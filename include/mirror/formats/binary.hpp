#pragma once

#include <mirror/codec/value.hpp>

#include <string>
#include <string_view>

namespace mirror::binary
{

std::string write(const mirror::value& input);
mirror::value read(std::string_view input);

} // namespace mirror::binary
