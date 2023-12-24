#include <cassert>
#include <cctype>
#include <compare>
#include <queue>
#include <ranges>
#include <string_view>

#include <fmt/format.h>

#include "algorithms.h"

template<class T>
struct TVec2
{
    T X;
    T Y;

    auto operator<=>(const TVec2&) const = default;

    TVec2 operator+(const T& rhs) const
    {
        return TVec2{ X + rhs, Y + rhs };
    }
    TVec2 operator*(const T& rhs) const
    {
        return TVec2{ X * rhs, Y * rhs };
    }
    TVec2 operator+(const TVec2& rhs) const
    {
        return TVec2{ X + rhs.X, Y + rhs.Y };
    }
    TVec2 operator-(const TVec2& rhs) const
    {
        return TVec2{ X - rhs.X, Y - rhs.Y };
    }
};

template<class T>
struct TVec3
{
    T X;
    T Y;
    T Z;

    auto operator<=>(const TVec3&) const = default;

    TVec3 operator+(const T& rhs) const
    {
        return TVec3{ X + rhs, Y + rhs, Z + rhs };
    }
    TVec3 operator*(const T& rhs) const
    {
        return TVec3{ X * rhs, Y * rhs, Z * rhs };
    }
    TVec3 operator+(const TVec3& rhs) const
    {
        return TVec3{ X + rhs.X, Y + rhs.Y, Z + rhs.Z };
    }
    TVec3 operator-(const TVec3& rhs) const
    {
        return TVec3{ X - rhs.X, Y - rhs.Y, Z - rhs.Z };
    }
};

using DVec2 = TVec2<double>;
using DVec3 = TVec3<double>;
using Vec2 = TVec2<int64_t>;
using Vec3 = TVec3<int64_t>;

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

    static constexpr auto project_velocity = [](const Vec3& v, bool project_xz)
    {
        return project_xz
                   ? DVec2{ static_cast<double>(v.X), static_cast<double>(v.Z) }
                   : DVec2{ static_cast<double>(v.X), static_cast<double>(v.Y) };
    };
    static constexpr auto get_time_at_intersection = [](const Hail& hail, const Vec3& dv, const Intersection& intersection)
    {
        // Transform hail into stones reference-frame
        const DVec2 v{ project_velocity(hail.Vel - dv, false) };

        if (hail.Vel.Y == dv.Y)
        {
            return static_cast<double>(intersection.At.X - hail.Pos.X) / v.X;
        }
        else
        {
            return static_cast<double>(intersection.At.Y - hail.Pos.Y) / v.Y;
        }
    };
    static constexpr auto get_intersection = [](const Hail& lhs, const Hail& rhs, const Vec3& dv, bool project_xz) -> std::optional<Intersection>
    {
        // Transform hail into stones reference-frame
        const DVec2 lhs_vel{ project_velocity(lhs.Vel - dv, project_xz) };
        const DVec2 rhs_vel{ project_velocity(rhs.Vel - dv, project_xz) };

        const double lhs_m{ lhs_vel.Y / lhs_vel.X };
        const double rhs_m{ rhs_vel.Y / rhs_vel.X };
        if (lhs_m != lhs_m || rhs_m != rhs_m)
        {
            // NaN
            // o.0
            return std::nullopt;
        }
        if (lhs_m == rhs_m)
        {
            // Parallel
            return std::nullopt;
        }

        const DVec2 lhs_pos{ project_velocity(lhs.Pos, project_xz) };
        const DVec2 rhs_pos{ project_velocity(rhs.Pos, project_xz) };

        double x_i{};
        double y_i{};
        if (std::abs(lhs_m) == std::numeric_limits<double>::infinity())
        {
            // lhs is vertical
            x_i = lhs_pos.X;
            y_i = rhs_m * (x_i - rhs_pos.X) + rhs_pos.Y;
        }
        else if (std::abs(rhs_m) == std::numeric_limits<double>::infinity())
        {
            // rhs is vertical
            x_i = rhs_pos.X;
            y_i = lhs_m * (x_i - lhs_pos.X) + lhs_pos.Y;
        }
        else
        {
            // Solve for x:
            //      y_1 = y_10 + m_1 * (x - x_10)
            //      y_2 = y_20 + m_2 * (x - x_20)
            //      y_1 = y_2
            x_i = (lhs_pos.Y - rhs_pos.Y - lhs_pos.X * lhs_m + rhs_pos.X * rhs_m) / (rhs_m - lhs_m);
            y_i = lhs_pos.Y + lhs_m * (x_i - lhs_pos.X);

            // Solve for x:
            //      x_1 = x_10 + (y - y_10) / m_1
            //      x_2 = x_20 + (y - y_20) / m_2
            //      x_1 = x_2
            // y_i = (lhs_pos.X - rhs_pos.X + lhs_m * rhs_pos.Y - rhs_m * lhs_pos.Y) / (lhs_m - rhs_m);
            // x_i = lhs_pos.X + (y_i - lhs_pos.Y) / lhs_m;
        }

        // Note: Has to fit into an int64, but it maybe still wrong :(
        Intersection intersection{
            {
                static_cast<int64_t>(std::round(x_i)),
                static_cast<int64_t>(std::round(y_i)),
            },
        };

        const double lhs_t{ get_time_at_intersection(lhs, dv, intersection) };
        const double rhs_t{ get_time_at_intersection(rhs, dv, intersection) };

        if (lhs_t < 0 || rhs_t < 0)
        {
            return std::nullopt;
        }

        // Note: Recalc from t, because the other might be wrong
        intersection.At.X = lhs.Pos.X + (lhs.Vel.X - dv.X) * static_cast<int64_t>(std::round(lhs_t));
        intersection.At.Y = lhs.Pos.Y + (lhs.Vel.Y - dv.Y) * static_cast<int64_t>(std::round(lhs_t));

        return intersection;
    };
    static constexpr auto get_stone_z_vel_from_intersection = [](const Hail& lhs, const Hail& rhs, const Vec3& dv, const Intersection& intersection)
    {
        const double lhs_vz{ static_cast<double>(lhs.Vel.Z) };
        const double rhs_vz{ static_cast<double>(rhs.Vel.Z) };
        const double lhs_z{ static_cast<double>(lhs.Pos.Z) };
        const double rhs_z{ static_cast<double>(rhs.Pos.Z) };
        const double lhs_t{ get_time_at_intersection(lhs, dv, intersection) };
        const double rhs_t{ get_time_at_intersection(rhs, dv, intersection) };
        return static_cast<int64_t>((lhs_z - rhs_z + lhs_t * lhs_vz - rhs_t * rhs_vz) / (lhs_t - rhs_t));
    };

    const std::string_view input_file{ argv[1] };
    const std::string file_data{ algo::read_whole_file(input_file) };

    const std::vector hail{ file_data | lines | to_string_views | to_hails | to_vector };

    // Loop until the end of time (hopefully not tho)
    for (const int64_t I : std::views::iota(int64_t{ 0 }))
    {
        // Find an xy-velocity for which all hail hits the stone
        //  Do this by transforming all the hail into the reference frame of the stone and finding
        //  a velocity for which all hail points intersect at the same point in that reference frame.
        //  That point will then be the origin of the stone, the velocity doesn't really matter for
        //  the solution but we have that too at that point.
        for (const int64_t X : std::views::iota(int64_t{ 0 }, I))
        {
            const int64_t Y{ X - I }; // No need to loop all of Y, there's too many redundant values
            for (const Vec3& stone_vel : std::array{
                     Vec3{ +X, +Y, 0 },
                     Vec3{ -X, +Y, 0 },
                     Vec3{ +X, -Y, 0 },
                     Vec3{ -X, -Y, 0 },
                 })
            {
                // Check if first two hails interesect
                if (const std::optional first_intersection_xy{ get_intersection(hail[0], hail[1], stone_vel, false) })
                {
                    // Then verify that all of the hail intersect at the same position
                    const bool solves_xy{
                        algo::all_of(hail | std::views::drop(2),
                                     [&](const Hail& rhs)
                                     {
                                         const std::optional this_intersection_xy{ get_intersection(hail[0], rhs, stone_vel, false) };
                                         if (!this_intersection_xy.has_value() || first_intersection_xy->At != this_intersection_xy->At)
                                         {
                                             return false;
                                         }
                                         return true;
                                     })
                    };
                    if (!solves_xy)
                    {
                        continue;
                    }

                    {
                        const Vec2 pos{
                            first_intersection_xy->At.X,
                            first_intersection_xy->At.Y,
                        };
                        fmt::print("Potential solution: {}, {} @ {}, {}\n",
                                   pos.X,
                                   pos.Y,
                                   stone_vel.X,
                                   stone_vel.Y);
                    }

                    // Compote what velocity the stone would've had for the two hail to intersect
                    //  at this position.
                    const int64_t Z{ get_stone_z_vel_from_intersection(
                        hail[0], hail[1], stone_vel, first_intersection_xy.value()) };
                    const Vec3& stone_vel_with_z{ stone_vel.X, stone_vel.Y, Z };

                    // Verify that the same velocity would work for all other hails
                    const bool solves_xz{
                        algo::all_of(hail | std::views::drop(2),
                                     [&](const Hail& rhs)
                                     {
                                         const int64_t this_z{ get_stone_z_vel_from_intersection(
                                             hail[0], rhs, stone_vel, first_intersection_xy.value()) };
                                         if (this_z != Z)
                                         {
                                             return false;
                                         }
                                         return true;
                                     })
                    };
                    if (!solves_xz)
                    {
                        continue;
                    }

                    // Here we found a velocity that solves our problem  \o.o/
                    //  all that is left is to compute the time at which the first stone hits the original
                    //  intersection in the XY-plane and back-compute the position of the stone
                    const double t{
                        get_time_at_intersection(hail[0], stone_vel_with_z, first_intersection_xy.value())
                    };
                    const Vec3 pos{
                        first_intersection_xy->At.X,
                        first_intersection_xy->At.Y,
                        hail[0].Pos.Z + static_cast<int64_t>(t) * (hail[0].Vel.Z - Z),
                    };
                    const Vec3 vel{ stone_vel_with_z };
                    fmt::print("Found solution: {}, {}, {} @ {}, {}, {}\n",
                               pos.X,
                               pos.Y,
                               pos.Z,
                               vel.X,
                               vel.Y,
                               vel.Z);

                    const int64_t init_pos_sum{ pos.X + pos.Y + pos.Z };
                    fmt::print("The result is: {}", init_pos_sum);
                    return init_pos_sum != 886858737029295;
                }
            }
        }
    }
    std::unreachable();
}
