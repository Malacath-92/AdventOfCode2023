#include <cctype>
#include <compare>
#include <queue>
#include <ranges>
#include <string_view>

#include <fmt/format.h>

#include "algorithms.h"

struct Vec2
{
    int64_t X;
    int64_t Y;

    auto operator<=>(const Vec2&) const = default;

    Vec2 operator+(const Vec2& rhs) const
    {
        return Vec2{ X + rhs.X, Y + rhs.Y };
    }
};

struct Hike
{
    std::vector<Vec2> Path;
    Vec2 Position;

    auto operator<=>(const Hike& rhs) const
    {
        return Path.size() <=> rhs.Path.size();
    }
};

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        fmt::print("Usage is exactly: *.exe input.txt");
        return 1;
    }

    static constexpr auto to_vector{ std::ranges::to<std::vector>() };
    static constexpr auto lines{ std::views::split('\n') };
    static constexpr auto to_string_views{ std::views::transform(
        [](auto str)
        { return std::string_view(str.data(), str.size()); }) };

    const std::string_view input_file{ argv[1] };
    const std::string file_data{ algo::read_whole_file(input_file) };

    const std::vector landscape{ file_data | lines | to_string_views | to_vector };

    const Vec2 landscape_min{ 0, 0 };
    const Vec2 landscape_max{ static_cast<int64_t>(landscape.front().size()), static_cast<int64_t>(landscape.size()) };
    const auto out_of_bounds = [&](const auto& coord)
    {
        const auto& [x, y] = coord;
        return x < landscape_min.X ||
               y < landscape_min.Y ||
               x >= landscape_max.X ||
               y >= landscape_max.Y;
    };

    std::priority_queue<Hike, std::vector<Hike>, std::greater<Hike>> hikes{};
    hikes.push({ {}, { 1, 0 } });

    std::optional<std::vector<Vec2>> maximum_path{ std::nullopt };
    while (!hikes.empty())
    {
        const Hike hike{ hikes.top() };
        hikes.pop();

        const auto& [x, y] = hike.Position;

        const bool on_target{
            x == landscape_max.X - 2 &&
            y == landscape_max.Y - 1
        };
        if (on_target)
        {
            if (!maximum_path.has_value() || hike.Path.size() > maximum_path->size())
            {
                maximum_path = std::move(hike.Path);
            }
            continue;
        }

        const auto try_push = [&](Vec2 p, Vec2 d, const auto& path)
        {
            p = p + d;
            if (out_of_bounds(p))
            {
                return;
            }

            if (algo::contains(path, p))
            {
                return;
            }

            const auto tile{ landscape[p.Y][p.X] };
            if (tile == '#')
            {
                return;
            }

            auto path_cpy{ path };
            path_cpy.push_back(p);

            hikes.push(Hike{ std::move(path_cpy), p });
        };

        using namespace std::string_view_literals;

        const auto tile{ landscape[y][x] };
        if (".>"sv.contains(tile))
        {
            try_push(hike.Position, Vec2{ +1, 0 }, hike.Path);
        }
        if (".<"sv.contains(tile))
        {
            try_push(hike.Position, Vec2{ -1, 0 }, hike.Path);
        }
        if (".v"sv.contains(tile))
        {
            try_push(hike.Position, Vec2{ 0, +1 }, hike.Path);
        }
        if (".^"sv.contains(tile))
        {
            try_push(hike.Position, Vec2{ 0, -1 }, hike.Path);
        }
    }

    if (!maximum_path.has_value())
    {
        fmt::print("No path found...");
        return 1;
    }

    const size_t maximum_path_length{ maximum_path->size() };
    fmt::print("The result is: {}", maximum_path_length);
    return maximum_path_length != 1017;
}
