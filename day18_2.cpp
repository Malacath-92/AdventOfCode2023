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

    std::vector<Vec2> vertices;
    int64_t on_edge{ 0 };

    {
        Vec2 current_pos{ 0, 0 };
        for (const auto& inst : instructions)
        {
            const Vec2 next_pos{ current_pos + c_Directions[size_t(inst.Dir)] * inst.Dist };
            vertices.push_back(next_pos);
            on_edge += inst.Dist;
            current_pos = next_pos;
        }
    }

    static constexpr auto area_by_shoelace = [](const auto& vertices)
    {
        int64_t area{ 0 };
        size_t j{ vertices.size() - 1 };
        for (size_t i = 0; i < vertices.size(); i++)
        {
            area += (vertices[j].X + vertices[i].X) * (vertices[j].Y - vertices[i].Y);
            j = i;
        }
        return std::abs(area / 2);
    };

    const int64_t total_volume{ area_by_shoelace(vertices) + on_edge / 2 + 1 };
    fmt::print("The result is: {}", total_volume);
    return total_volume != 129849166997110;
}
