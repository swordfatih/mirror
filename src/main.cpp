
#include <meta>

#include <spdlog/spdlog.h>

import std;
import mirror.json;
import mirror.reflect;
import mirror.serialization;
import mirror.value;
import mirror.yaml;

struct Point
{
    int x;
    int y;
};

struct User
{
    std::string            name;
    std::int32_t           age;
    Point                  home;
    std::unique_ptr<Point> point;
};

int main()
{
    Point point{23, 67};

    User user{"fatih", 23, point, std::make_unique<Point>(point)};

    const auto tree = mirror::serialize(user);

    const auto json_document = mirror::json::write(tree);
    spdlog::info("json document: {}", json_document);

    const auto yaml_document = mirror::yaml::write(tree);
    spdlog::info("yaml document:\n{}", yaml_document);

    const auto parsed_user = mirror::deserialize<User>(mirror::json::read(json_document));
    spdlog::info("parsed user JSON: {}", mirror::json::write(mirror::serialize(parsed_user)));

    const auto parsed_yaml_user = mirror::deserialize<User>(mirror::yaml::read(yaml_document));
    spdlog::info("parsed YAML user JSON: {}", mirror::json::write(mirror::serialize(parsed_yaml_user)));
}
