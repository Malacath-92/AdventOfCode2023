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

enum class Direction : uint8_t
{
    Right,
    Down,
    Left,
    Up,
};

inline constexpr Vec2 c_Directions[]{
    { +1, 0 },
    { 0, +1 },
    { -1, 0 },
    { 0, -1 },
};

struct Instruction
{
    Direction Dir;
    int64_t Dist;
};

struct Segment
{
    Vec2 From;
    Vec2 To;

    auto operator<=>(const Segment&) const = default;
};

struct Intersection
{
    int64_t X;
    char C;

    auto operator<=>(const Intersection&) const = default;
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

    //static constexpr auto to_direction =
    //    [](char c)
    //{
    //    using enum Direction;
    //    switch (c)
    //    {
    //    case 'R':
    //        return Right;
    //    case 'L':
    //        return Left;
    //    case 'D':
    //        return Down;
    //    case 'U':
    //    default:
    //        return Up;
    //    }
    //};
    //static constexpr auto to_instructions{ std::views::transform(
    //    [](auto str)
    //    {
    //        const std::vector parts{ str | std::views::split(' ') | to_string_views | to_vector };

    //        return Instruction{
    //            to_direction(parts[0][0]),
    //            algo::stoi<int64_t>(parts[1]),
    //        };
    //    }) };
     static constexpr auto to_direction =
         [](char c)
    {
         return static_cast<Direction>(c - '0');
     };
     static constexpr auto to_instructions{ std::views::transform(
         [](auto str)
         {
             using namespace std::string_view_literals;
             const std::vector parts{ str | std::views::split(' ') | to_string_views | to_vector };
             const std::string_view col{ algo::trim(parts[2], "#()"sv) };
             // Should be using from_chars or make my own stoi taking a base, but time is of the essence
             uint32_t dist{ std::stoul(std::string{ col.substr(0, 5) }, nullptr, 16) };
    
             return Instruction{
                 to_direction(col.back()),
                 dist,
             };
         }) };

    const std::string_view input_file{ argv[1] };
    const std::string file_data{ algo::read_whole_file(input_file) };

    const std::vector instructions{ file_data | lines | to_string_views | to_instructions | to_vector };

    std::vector<Segment> outline;
    Vec2 min{ 0, 0 };
    Vec2 max{ 0, 0 };

    {
        Vec2 current_pos{ 0, 0 };
        for (const auto& inst : instructions)
        {
            const Vec2 next_pos{ current_pos + c_Directions[size_t(inst.Dir)] * inst.Dist };
            outline.push_back({ current_pos, next_pos });
            current_pos = next_pos;

            min.X = std::min(current_pos.X, min.X);
            min.Y = std::min(current_pos.Y, min.Y);
            max.X = std::max(current_pos.X, max.X);
            max.Y = std::max(current_pos.Y, max.Y);
        }
    }

    int64_t total_volume{ 0 };
    for (int64_t y = min.Y; y <= max.Y; y++)
    {
        std::set<Intersection> intersections{};
        for (const auto& out : outline)
        {
            const Vec2 out_min{ std::min(out.From.X, out.To.X), std::min(out.From.Y, out.To.Y) };
            const Vec2 out_max{ std::max(out.From.X, out.To.X), std::max(out.From.Y, out.To.Y) };
            if (out_min.X == out_max.X && out_min.Y <= y && out_max.Y >= y)
            {
                char c;
                if (y != out_min.Y && y != out_max.Y)
                {
                    c = out_min.Y == out.From.Y ? 'V' : 'A';
                }
                else
                {
                    c = out_min.Y == out.From.Y ? 'Y' : '^';
                }
                intersections.insert({ out_min.X, c });
            }
        }

        int64_t current_x{ min.X };
        int64_t inside{ false };
        std::optional<Segment> prev_seg{};
        for (auto [x, c] : intersections)
        {
            Segment seg{ Vec2{ current_x, y }, Vec2{ x, y } };
            if (inside != 0)
            {
                const bool no_double_count{ prev_seg.has_value() && prev_seg->To == seg.From };
                prev_seg = seg;
                const int64_t add{ x - current_x + (no_double_count ? 0 : 1) };
                total_volume += add;
            }
            else
            {
                prev_seg.reset();
            }

            switch (c)
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

            current_x = x;
        }
    }

    fmt::print("The result is: {}", total_volume);
    return total_volume != 129849166997110;
}
