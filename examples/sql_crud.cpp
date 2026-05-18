#include <cstdint>
#include <optional>
#include <string>

#include <spdlog/spdlog.h>

#include <mirror/extensions/sql.hpp>
#include <mirror/mirror.hpp>

struct User
{
    std::uint64_t              id = 0;
    std::string                name;
    std::optional<std::string> email;
};

template <>
struct mirror::sql::table<User>
{
    static constexpr std::string_view name = "users";
    static constexpr std::string_view primary_key = "id";
};

int main()
{
    const User user{
        .id = 42,
        .name = "username",
        .email = std::nullopt,
    };

    const auto insert = mirror::sql::insert(user);
    const auto update = mirror::sql::update(user);
    const auto select = mirror::sql::select_by_id<User>(user.id);
    const auto remove = mirror::sql::delete_by_id<User>(user.id);

    spdlog::info("insert: {} [{} binds]", insert.text, insert.binds.size());
    spdlog::info("update: {} [{} binds]", update.text, update.binds.size());
    spdlog::info("select: {} [{} binds]", select.text, select.binds.size());
    spdlog::info("delete: {} [{} binds]", remove.text, remove.binds.size());
}
