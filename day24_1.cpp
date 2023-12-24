#include <cassert>
#include <cctype>
#include <compare>
#include <queue>
#include <ranges>
#include <string_view>

#include <fmt/format.h>

#include "algorithms.h"

struct DVec2
{
    double X;
    double Y;

    auto operator<=>(const DVec2&) const = default;

    DVec2 operator+(const int64_t& rhs) const
    {
        return DVec2{ X + rhs, Y + rhs };
    }
    DVec2 operator*(const int64_t& rhs) const
    {
        return DVec2{ X * rhs, Y * rhs };
    }
    DVec2 operator+(const DVec2& rhs) const
    {
        return DVec2{ X + rhs.X, Y + rhs.Y };
    }
    DVec2 operator-(const DVec2& rhs) const
    {
        return DVec2{ X - rhs.X, Y - rhs.Y };
    }
};

struct Vec2
{
    int64_t X;
    int64_t Y;

    auto operator<=>(const Vec2&) const = default;

    Vec2 operator+(const int64_t& rhs) const
    {
        return Vec2{ X + rhs, Y + rhs };
    }
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
};

struct Vec3
{
    int64_t X;
    int64_t Y;
    int64_t Z;

    auto operator<=>(const Vec3&) const = default;

    Vec3 operator+(const int64_t& rhs) const
    {
        return Vec3{ X + rhs, Y + rhs, Z + rhs };
    }
    Vec3 operator*(const int64_t& rhs) const
    {
        return Vec3{ X * rhs, Y * rhs, Z * rhs };
    }
    Vec3 operator+(const Vec3& rhs) const
    {
        return Vec3{ X + rhs.X, Y + rhs.Y, Z + rhs.Z };
    }
    Vec3 operator-(const Vec3& rhs) const
    {
        return Vec3{ X - rhs.X, Y - rhs.Y, Z - rhs.Z };
    }
};

struct Hail
{
    Vec3 Pos;
    Vec3 Vel;
};

struct Intersection
{
    Vec2 At;
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
        { return algo::trim(std::string_view(str.data(), str.size())); }) };

    static constexpr auto to_numbers{ std::views::transform(
        [](auto str)
        {
            return algo::stoi<int64_t>(str);
        }) };
    static constexpr auto to_vecs{ std::views::transform(
        [](auto str)
        {
            const std::vector coords{ str | std::views::split(',') | to_string_views | to_numbers | to_vector };
            return Vec3{ coords[0], coords[1], coords[2] };
        }) };
    static constexpr auto to_hails{ std::views::transform(
        [](auto str)
        {
            const std::vector vecs{ str | std::views::split('@') | to_string_views | to_vecs | to_vector };
            return Hail{ vecs[0], vecs[1] };
        }) };

    static constexpr auto get_intersection = [](const Hail& lhs, const Hail& rhs) -> std::optional<Intersection>
    {
        const DVec2 lhs_vel{ static_cast<double>(lhs.Vel.X), static_cast<double>(lhs.Vel.Y) };
        const DVec2 rhs_vel{ static_cast<double>(rhs.Vel.X), static_cast<double>(rhs.Vel.Y) };

        const double lhs_m{ lhs_vel.Y / lhs_vel.X };
        const double rhs_m{ rhs_vel.Y / rhs_vel.X };
        if (lhs_m == rhs_m)
        {
            // Parallel
            return std::nullopt;
        }

        const DVec2 lhs_pos{ static_cast<double>(lhs.Pos.X), static_cast<double>(lhs.Pos.Y) };
        const DVec2 rhs_pos{ static_cast<double>(rhs.Pos.X), static_cast<double>(rhs.Pos.Y) };

        double x_i{};
        double y_i{};
        double lhs_t{};
        double rhs_t{};
        if (lhs_m == std::numeric_limits<double>::infinity())
        {
            // lhs is vertical
            x_i = lhs_pos.X;
            y_i = rhs_m * (x_i - rhs_pos.X) + rhs_pos.Y;
            lhs_t = (y_i - lhs_pos.Y) / lhs_vel.Y;
            rhs_t = (y_i - rhs_pos.Y) / rhs_vel.Y;
        }
        else if (lhs_m == std::numeric_limits<double>::infinity())
        {
            // rhs is vertical
            x_i = rhs_pos.X;
            y_i = lhs_m * (x_i - lhs_pos.X) + lhs_pos.Y;
            lhs_t = (x_i - lhs_pos.X) / lhs_vel.X;
            rhs_t = (x_i - rhs_pos.X) / rhs_vel.X;
        }
        else
        {
            // Solve: y = y0 + m * (x - x0) for x
            x_i = (lhs_pos.Y - rhs_pos.Y - lhs_pos.X * lhs_m + rhs_pos.X * rhs_m) / (rhs_m - lhs_m);
            y_i = lhs_pos.Y + lhs_m * (x_i - lhs_pos.X);
            lhs_t = (x_i - lhs_pos.X) / lhs_vel.X;
            rhs_t = (x_i - rhs_pos.X) / rhs_vel.X;
        }

        if (lhs_t < 0 || rhs_t < 0)
        {
            return std::nullopt;
        }

        // Note: Has to fit into an int64
        return {
            {
                static_cast<int64_t>(x_i),
                static_cast<int64_t>(y_i),
            },
        };
    };

    const std::string_view input_file{ argv[1] };
    const std::string file_data{ algo::read_whole_file(input_file) };

    const std::vector hail{ file_data | lines | to_string_views | to_hails | to_vector };

    const Vec2 test_min{ 200000000000000, 200000000000000 };
    const Vec2 test_max{ 400000000000000, 400000000000000 };

    size_t num_intersections{ 0 };
    for (size_t i = 0; i < hail.size(); i++)
    {
        const Hail& lhs{ hail[i] };
        for (size_t j = i + 1; j < hail.size(); j++)
        {
            const Hail& rhs{ hail[j] };
            if (const std::optional intersection{ get_intersection(lhs, rhs) })
            {
                if (intersection->At.X >= test_min.X &&
                    intersection->At.Y >= test_min.Y &&
                    intersection->At.X <= test_max.X &&
                    intersection->At.Y <= test_max.Y)
                {
                    num_intersections++;
                }
            }
        }
    }

    fmt::print("The result is: {}", num_intersections);
    return num_intersections != 18098;
}
