#include <array>
#include <meta>

#include <spdlog/spdlog.h>

import std;

struct Point
{
    int x;
    int y;

    std::string to_string()
    {
        return std::format("({} {})", x, y);
    }
};

consteval std::meta::info member_number(int n)
{
    auto ctx = std::meta::access_context::current();
    return std::meta::nonstatic_data_members_of(^^Point, ctx)[n];
}

template <typename Type>
consteval std::size_t member_count()
{
    auto ctx = std::meta::access_context::current();
    return std::meta::members_of(^^Type, ctx).size();
}

template <typename Type>
consteval auto members()
{
    auto             ctx = std::meta::access_context::current();
    constexpr size_t N = member_count<Type>();

    auto members = std::meta::members_of(^^Type, ctx);

    std::array<std::string_view, N> names{};

    for(size_t i = 0; i < N; ++i)
    {
        if(std::meta::has_identifier(members[i]))
        {
            auto name = std::meta::identifier_of(members[i]);
            if(!name.empty())
            {
                names[i] = std::meta::identifier_of(members[i]);
            }
        }
    }

    return names;
}

int main()
{
    spdlog::info("hello");

    constexpr std::meta::info r_Point = ^^Point;
    std::string_view          name_Point = std::meta::identifier_of(r_Point);

    typename[:r_Point:] p1 = {1, 2};
    constexpr auto r_p1 = ^^p1;

    spdlog::info("{} {} {}", name_Point, std::meta::identifier_of(r_p1), p1.to_string());

    using Point2 = [:r_Point:];
    Point2 p2 = {3, 4};

    spdlog::info("{}", p2.to_string());

    p2.[:member_number(0):] = 23;
    spdlog::info("{}", p2.to_string());

    constexpr auto names = members<std::array<int, 5>>();
    for(auto member: names)
    {
        spdlog::info("{}", member);
    }
}
