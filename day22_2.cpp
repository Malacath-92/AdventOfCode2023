#include <cassert>
#include <cctype>
#include <compare>
#include <queue>
#include <ranges>
#include <string_view>

#include <fmt/format.h>

#include "algorithms.h"

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

    Vec3 normalized() const
    {
        if (X == 0 && Y == 0 && Z == 0)
        {
            return Vec3{ 0, 0, 1 };
        }
        static constexpr auto sign = [](auto i)
        {
            return i == 0  ? 0
                   : i > 0 ? +1
                           : -1;
        };
        // We only have one non-zero direction
        return Vec3{ sign(X), sign(Y), sign(Z) };
    }
};

template<class T>
struct FlatMultiArrayView
{
    int64_t Width;
    int64_t Depth;
    int64_t Height;
    std::span<T> Data;

    template<class SelfT>
    auto operator[](this SelfT& self, int64_t x)
    {
        struct IndexProxyX
        {
            SelfT& Self;
            int64_t X;

            auto operator[](int64_t y)
            {
                struct IndexProxyXY
                {
                    SelfT& Self;
                    int64_t X;
                    int64_t Y;

                    auto& operator[](int64_t z)
                    {
                        return Self.Data[X + Y * Self.Width + z * Self.Width * Self.Depth];
                    }
                };

                return IndexProxyXY{ Self, X, y };
            }
        };

        return IndexProxyX{ self, x };
    }
};

struct Brick
{
    Vec3 From;
    Vec3 To;

    std::set<size_t> Supports;
    std::set<size_t> SupportedBy;
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
    static constexpr auto to_bricks{ std::views::transform(
        [](auto str)
        {
            const std::vector vecs{ str | std::views::split('~') | to_string_views | to_vecs | to_vector };
            [[maybe_unused]] const auto dx{ vecs[0].X - vecs[1].X };
            [[maybe_unused]] const auto dy{ vecs[0].Y - vecs[1].Y };
            [[maybe_unused]] const auto dz{ vecs[0].Z - vecs[1].Z };
            assert((dx == 0 && dy == 0) || (dx == 0 && dz == 0) || (dy == 0 && dz == 0));
            return Brick{ vecs[0], vecs[1] };
        }) };

    const std::string_view input_file{ argv[1] };
    const std::string file_data{ algo::read_whole_file(input_file) };

    std::vector bricks{ file_data | lines | to_string_views | to_bricks | to_vector };

    const Vec3 world_min{ 0, 0, 0 };
    const Vec3 world_max{
        [&bricks]()
        {
            Vec3 world_max{ 0, 0, 0 };
            for (const Brick& brick : bricks)
            {
                world_max.X = algo::max(brick.From.X, brick.To.X, world_max.X);
                world_max.Y = algo::max(brick.From.Y, brick.To.Y, world_max.Y);
                world_max.Z = algo::max(brick.From.Z, brick.To.Z, world_max.Z);
            }
            return world_max;
        }()
    };

    static constexpr size_t c_Air{ 0xfefefefefefefefe };
    static constexpr size_t c_Ground{ 0xffffffffffffffff };

    const Vec3 world_data_size{ world_max + 1 };
    std::vector world_data(world_data_size.X * world_data_size.Y * world_data_size.Z, size_t{ c_Air });
    FlatMultiArrayView<size_t> world{ world_data_size.X, world_data_size.Y, world_data_size.Z, world_data };

    // Initialize ground
    for (int64_t x = 0; x <= world_max.X; x++)
    {
        for (int64_t y = 0; y <= world_max.Y; y++)
        {
            world[x][y][0] = c_Ground;
        }
    }

    // Initialize bricks in world
    for (const auto& [i, brick] : std::views::enumerate(bricks))
    {
        const Vec3 dir{ (brick.To - brick.From).normalized() };
        Vec3 pos{ brick.From };
        while (pos != brick.To + dir)
        {
            const auto& [x, y, z] = pos;
            world[x][y][z] = i;
            pos = pos + dir;
        }
    }

    // Update bricks until they all settle
    const auto settle = [&]()
    {
        while (true)
        {
            std::vector settled(bricks.size(), uint8_t{ 0 });
            for (const auto& [i, brick] : std::views::enumerate(bricks))
            {
                const Vec3 from{ brick.From.X, brick.From.Y, std::min(brick.From.Z, brick.To.Z) };
                const Vec3 to{ brick.To.X, brick.To.Y, std::max(brick.To.Z, brick.From.Z) };
                const Vec3 dir{ (to - from).normalized() };
                const Vec3 down{ 0, 0, -1 };

                bool blocked{ false };
                {
                    // Check if we cab move
                    Vec3 pos{ brick.From };
                    while (pos != to + dir)
                    {
                        [[maybe_unused]] const auto here{ world[pos.X][pos.Y][pos.Z] };
                        assert(here == static_cast<size_t>(i));

                        const Vec3 below{ pos + down };
                        const size_t i_below{ world[below.X][below.Y][below.Z] };
                        if (i_below == static_cast<size_t>(i))
                        {
                            break;
                        }
                        else if (i_below != c_Air)
                        {
                            blocked = true;
                            break;
                        }
                        pos = pos + dir;
                    }
                }

                if (!blocked)
                {
                    Vec3 pos{ brick.From };
                    while (pos != to + dir)
                    {
                        const Vec3 below{ pos + down };

                        [[maybe_unused]] const auto here{ world[pos.X][pos.Y][pos.Z] };
                        assert(here == static_cast<size_t>(i));
                        [[maybe_unused]] const auto there{ world[below.X][below.Y][below.Z] };
                        assert(there == c_Air);

                        world[pos.X][pos.Y][pos.Z] = c_Air;
                        world[below.X][below.Y][below.Z] = i;
                        pos = pos + dir;
                    }
                    assert(brick.From.Z > 1);
                    assert(brick.To.Z > 1);
                    bricks[i] = { brick.From + down, brick.To + down };
                }

                settled[i] = blocked;
            }
            if (algo::all_of(settled))
            {
                return;
            }
        }
    };

    const auto print_xz = [&]()
    {
        std::string str;
        for (int64_t zz = 0; zz <= world_max.Z; zz++)
        {
            const int64_t z = world_max.Z - zz;
            for (int64_t x = 0; x <= world_max.X; x++)
            {
                char c{ ' ' };
                for (int64_t y = 0; y <= world_max.Y; y++)
                {
                    const size_t i{ world[x][y][z] };
                    if (i == c_Ground)
                    {
                        c = '~';
                        break;
                    }
                    else if (i != c_Air)
                    {
                        size_t d{ 'A' + i };
                        if (d > 'Z')
                        {
                            d = (d - 'A') % ('Z' - 'A') + 'A';
                        }

                        char dd{ char(d) };
                        if (c == ' ')
                        {
                            c = dd;
                        }
                        else if (c != dd)
                        {
                            c = '?';
                            break;
                        }
                    }
                }
                str += c;
            }
            str += '\n';
        }
        fmt::print("{}\n", str);
    };
    const auto print_yz = [&]()
    {
        std::string str;
        for (int64_t zz = 0; zz <= world_max.Z; zz++)
        {
            const int64_t z = world_max.Z - zz;
            for (int64_t y = 0; y <= world_max.Y; y++)
            {
                char c{ ' ' };
                for (int64_t x = 0; x <= world_max.X; x++)
                {
                    const size_t i{ world[x][y][z] };
                    if (i == c_Ground)
                    {
                        c = '~';
                        break;
                    }
                    else if (i != c_Air)
                    {
                        size_t d{ 'A' + i };
                        if (d > 'Z')
                        {
                            d = (d - 'A') % ('Z' - 'A') + 'A';
                        }

                        char dd{ char(d) };
                        if (c == ' ')
                        {
                            c = dd;
                        }
                        else if (c != dd)
                        {
                            c = '?';
                            break;
                        }
                    }
                }
                str += c;
            }
            str += '\n';
        }
        fmt::print("{}\n", str);
    };

    print_xz();
    print_yz();
    settle();
    settle();
    print_xz();
    print_yz();

    // Mark which bricks are supported by which other bricks
    for (const auto& [i, brick] : std::views::enumerate(bricks))
    {
        const Vec3 from{ brick.From.X, brick.From.Y, std::min(brick.From.Z, brick.To.Z) };
        const Vec3 to{ brick.To.X, brick.To.Y, std::max(brick.To.Z, brick.From.Z) };
        const Vec3 dir{ (to - from).normalized() };
        const Vec3 up{ 0, 0, 1 };

        Vec3 pos{ brick.From };
        while (pos != to + dir)
        {
            [[maybe_unused]] const auto here{ world[pos.X][pos.Y][pos.Z] };
            assert(here == static_cast<size_t>(i));

            if (pos.Z < world_max.Z)
            {
                const Vec3 above{ pos + up };
                const size_t i_above{ world[above.X][above.Y][above.Z] };
                if (i_above != static_cast<size_t>(i) && i_above != c_Air)
                {
                    bricks[i].Supports.insert(i_above);
                    bricks[i_above].SupportedBy.insert(i);
                }
            }
            pos = pos + dir;
        }
    }

    const auto compute_num_falling_bricks = [](this const auto& self, size_t i, auto& bricks) -> size_t {
        auto& brick{ bricks[i] };

        size_t num_falling_bricks{};
        for (const auto& j : brick.Supports)
        {
            bricks[j].SupportedBy.erase(i);
            if (bricks[j].SupportedBy.empty())
            {
                num_falling_bricks += 1 + self(j, bricks);
            }
        }
        return num_falling_bricks;
    };

    size_t num_falling_bricks{};
    for (size_t i = 0; i < bricks.size(); i++)
    {
        auto bricks_cpy{ bricks };
        num_falling_bricks += compute_num_falling_bricks(i, bricks_cpy);
    }

    fmt::print("The result is: {}", num_falling_bricks);
    return num_falling_bricks != 3632;
}
