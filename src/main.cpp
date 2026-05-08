
#include <meta>

#include <spdlog/spdlog.h>

import std;
import mirror;

struct Point
{
    int x;
    int y;
};

struct User
{
    std::string name;
    int         age;
    Point       home;
};

int main()
{
    Point point{23, 67};
    spdlog::info("point: {}", mirror::to_string(point));

    User user{"fatih", 23, point};
    spdlog::info("user: {}", mirror::to_string(user));
}
