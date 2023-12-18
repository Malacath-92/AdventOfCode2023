#include <cctype>
#include <compare>
#include <queue>
#include <ranges>
#include <string_view>

#include <fmt/format.h>

#include "algorithms.h"

enum class Direction : uint8_t
{
    Up,
    Down,
    Left,
    Right,
};

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

inline constexpr Vec2 c_Directions[]{
    { 0, -1 },
    { 0, +1 },
    { -1, 0 },
    { +1, 0 },
};

struct Color
{
    uint8_t Red;
    uint8_t Green;
    uint8_t Blue;
};

struct Instruction
{
    Color Col;
    Direction Dir;
    int64_t Dist;
};

struct Outline
{
    Color Col;
    Vec2 From;
    Vec2 To;
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

    static constexpr auto to_color =
        [](auto str)
    {
        using namespace std::string_view_literals;
        // Should be using from_chars or make my own stoi taking a base, but time is of the essence
        uint32_t col{ std::stoul(std::string{ algo::trim(str, "#()"sv) }, nullptr, 16) };
        return reinterpret_cast<Color&>(col);
    };
    static constexpr auto to_direction =
        [](char c)
    {
        using enum Direction;
        switch (c)
        {
        case 'R':
            return Right;
        case 'L':
            return Left;
        case 'D':
            return Down;
        case 'U':
        default:
            return Up;
        }
    };
    static constexpr auto to_instructions{ std::views::transform(
        [](auto str)
        {
            const std::vector parts{ str | std::views::split(' ') | to_string_views | to_vector };

            return Instruction{
                to_color(parts[2]),
                to_direction(parts[0][0]),
                algo::stoi<int64_t>(parts[1]),
            };
        }) };

    const std::string_view input_file{ argv[1] };
    const std::string file_data{ algo::read_whole_file(input_file) };

    const std::vector instructions{ file_data | lines | to_string_views | to_instructions | to_vector };

    std::vector<Outline> outline;
    Vec2 min{ 0, 0 };
    Vec2 max{ 0, 0 };

    {
        Vec2 current_pos{ 0, 0 };
        for (const auto& inst : instructions)
        {
            const Vec2 next_pos{ current_pos + c_Directions[size_t(inst.Dir)] * inst.Dist };
            outline.push_back({ inst.Col, current_pos, next_pos });
            current_pos = next_pos;

            min.X = std::min(current_pos.X, min.X);
            min.Y = std::min(current_pos.Y, min.Y);
            max.X = std::max(current_pos.X, max.X);
            max.Y = std::max(current_pos.Y, max.Y);
        }
    }

    if (min.X != 0 || min.Y != 0)
    {
        for (auto& out : outline)
        {
            out.From = out.From - min;
            out.To = out.To - min;
        }

        max = max - min;
        min = min - min;
    }

    std::vector pool(max.Y + 1, std::vector(max.X + 1, ' '));
    std::vector border(max.Y + 1, std::vector(max.X + 1, ' '));
    for (const auto& out : outline)
    {
        const auto min_x{ std::min(out.From.X, out.To.X) };
        const auto max_x{ std::max(out.From.X, out.To.X) };
        const auto min_y{ std::min(out.From.Y, out.To.Y) };
        const auto max_y{ std::max(out.From.Y, out.To.Y) };
        for (auto x = min_x; x <= max_x; x++)
        {
            for (auto y = min_y; y <= max_y; y++)
            {
                pool[y][x] = '0';

                if (min_x != max_x)
                {
                    if (x != min_x && x != max_x)
                    {
                        border[y][x] = min_x == out.From.X
                                           ? '>'
                                           : '<';
                    }
                }
                else
                {
                    if (y != min_y && y != max_y)
                    {
                        border[y][x] = min_y == out.From.Y
                                           ? 'V'
                                           : 'A';
                    }
                    else
                    {
                        border[y][x] = min_y == out.From.Y
                                           ? 'Y'
                                           : '^';
                    }
                }
            }
        }
    }

    for (size_t y = 0; y < pool.size(); y++)
    {
        auto& row{ pool[y] };
        int64_t inside{ 0 };
        for (size_t x = 0; x < row.size(); x++)
        {
            auto& c{ row[x] };
            if (c != ' ')
            {
                const auto& b{ border[y][x] };
                switch (b)
                {
                case 'A':
                    inside -= 2;
                    break;
                case '^':
                    inside -= 1;
                    break;
                case 'V':
                    inside += 2;
                    break;
                case 'Y':
                    inside += 1;
                    break;
                default:
                    break;
                }
            }
            else if (inside != 0)
            {
                c = '0';
            }
        }
    }

    const int64_t total_volume{ algo::count(pool | std::views::join, '0') };
    fmt::print("The result is: {}", total_volume);
    return total_volume != 40714;
}
