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

    Vec2 operator*(const int64_t& rhs) const
    {
        return Vec2{ X * rhs, Y * rhs };
    }
    Vec2 operator+(const Vec2& rhs) const
    {
        return Vec2{ X + rhs.X, Y + rhs.Y };
    }
    Vec2 operator-(const Vec2& rhs) const
    {
        return Vec2{ X - rhs.X, Y - rhs.Y };
    }
    Vec2 rot_left() const
    {
        return Vec2{ Y, -X };
    }
    Vec2 rot_right() const
    {
        return Vec2{ -Y, X };
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

    const std::vector garden{ file_data | lines | to_string_views | to_vector };
    const Vec2 starting_pos{
        [&garden]()
        {
            for (size_t y = 0; y < garden.size(); y++)
            {
                for (size_t x = 0; x < garden.front().size(); x++)
                {
                    if (garden[y][x] == 'S')
                    {
                        return Vec2{
                            static_cast<int64_t>(x),
                            static_cast<int64_t>(y),
                        };
                    }
                }
            }
            throw "No starting pos...";
        }()
    };

    const Vec2 garden_min{ 0, 0 };
    const Vec2 garden_max{ static_cast<int64_t>(garden.front().size()), static_cast<int64_t>(garden.size()) };
    const auto out_of_bounds = [&](const auto& coord)
    {
        const auto& [x, y] = coord;
        return x < garden_min.X ||
               y < garden_min.Y ||
               x >= garden_max.X ||
               y >= garden_max.Y;
    };

    std::vector positions{ starting_pos };
    for (size_t i = 0; i < 64; i++)
    {
        std::vector<Vec2> next_positions;
        for (const auto& [x, y] : positions)
        {
            const Vec2 top{ x, y - 1 };
            const Vec2 bot{ x, y + 1 };
            const Vec2 lef{ x - 1, y };
            const Vec2 rig{ x + 1, y };

            const auto try_push = [&](Vec2 pos)
            {
                using namespace std::string_view_literals;
                if (!out_of_bounds(pos) &&
                    ".S"sv.contains(garden[pos.Y][pos.X]) &&
                    !algo::contains(next_positions, pos))
                {
                    next_positions.push_back(pos);
                }
            };
            try_push(top);
            try_push(bot);
            try_push(lef);
            try_push(rig);
        }
        std::swap(positions, next_positions);
    }

    size_t num_final_positions{ positions.size() };
    fmt::print("The result is: {}", num_final_positions);
    return num_final_positions != 3632;
}
