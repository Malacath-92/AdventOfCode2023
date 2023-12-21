#include <cassert>
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
    const Vec2 garden_min{ 0, 0 };
    const Vec2 garden_max{ static_cast<int64_t>(garden.front().size()), static_cast<int64_t>(garden.size()) };

    const Vec2 starting_pos{
        [&]()
        {
            for (int64_t x = 0; x < garden_max.X; x++)
            {
                for (int64_t y = 0; y < garden_max.Y; y++)
                {
                    if (garden[y][x] == 'S')
                    {
                        return Vec2{ x, y };
                    }
                }
            }
            throw "No starting pos...";
        }()
    };

    // Have to be in the exact center of an uneven-sized garden
    assert(starting_pos.X == starting_pos.Y);
    assert(starting_pos.X % 2 == 1);
    assert(garden_max.X == garden_max.Y);
    assert(garden_max.X % 2 == 1);
    assert(starting_pos.X == garden_max.X / 2);

    const auto modulo_bounds = [&](Vec2 coord)
    {
        const auto& [x, y] = coord;
        const auto& [X, Y] = garden_max;
        return std::pair{
            Vec2{
                (x % X + X) % X,
                (y % Y + Y) % Y,
            },
            Vec2{
                (x + (x < 0 ? 1 : 0)) / X - (x < 0 ? 1 : 0),
                (y + (y < 0 ? 1 : 0)) / Y - (y < 0 ? 1 : 0),
            },
        };
    };

    using WrappingTile = std::set<Vec2>;
    std::vector steps(garden_max.Y, std::vector(garden_max.X, WrappingTile{}));
    steps[starting_pos.Y][starting_pos.X].insert({ 0, 0 });

    static constexpr int64_t c_NumSteps{ 26501365 };
    const int64_t steps_to_fill_first_square{ garden_max.X };
    const int64_t num_multiples_of_first_square{ c_NumSteps / steps_to_fill_first_square };
    const int64_t num_extra_steps{ c_NumSteps % steps_to_fill_first_square };
    const int64_t num_explicit_steps{ 3 * steps_to_fill_first_square + num_extra_steps };

    const auto do_steps = [&](int64_t n)
    {
        for (int64_t i = 0; i < n; i++)
        {
            std::vector next_steps(garden_max.Y, std::vector(garden_max.X, WrappingTile{}));
            const auto step_to = [&](Vec2 tile, Vec2 to)
            {
                using namespace std::string_view_literals;

                const auto [mod_to, tile_to] = modulo_bounds(to);
                if (".S"sv.contains(garden[mod_to.Y][mod_to.X]))
                {
                    next_steps[mod_to.Y][mod_to.X].insert(tile + tile_to);
                }
            };

            for (int64_t x = 0; x < garden_max.X; x++)
            {
                for (int64_t y = 0; y < garden_max.Y; y++)
                {
                    const Vec2 cur{ x, y };
                    const Vec2 top{ x, y - 1 };
                    const Vec2 bot{ x, y + 1 };
                    const Vec2 lef{ x - 1, y };
                    const Vec2 rig{ x + 1, y };

                    for (const auto& tile : steps[y][x])
                    {
                        step_to(tile, top);
                        step_to(tile, bot);
                        step_to(tile, lef);
                        step_to(tile, rig);
                    }
                }
            }
            std::swap(steps, next_steps);
        }
    };
    do_steps(num_explicit_steps);

    const auto count_square = [&](Vec2 square_pos)
    {
        int64_t cnt{ 0 };
        for (int64_t x = 0; x < garden_max.X; x++)
        {
            for (int64_t y = 0; y < garden_max.Y; y++)
            {
                cnt += steps[y][x].contains(square_pos) ? 1 : 0;
            }
        }
        fmt::print("{{ {}, {} }} -> {}\n", square_pos.X, square_pos.Y, cnt);
        return cnt;
    };

    const int64_t filled_odd_square{ count_square({ 0, 0 }) };
    const int64_t filled_even_square{ count_square({ 0, 1 }) };
    assert(filled_even_square == count_square({ 0, -1 }));
    assert(filled_even_square == count_square({ 1, 0 }));
    assert(filled_even_square == count_square({ -1, 0 }));

    const int64_t top_square{ count_square({ 0, -3 }) };
    const int64_t bot_square{ count_square({ 0, +3 }) };
    const int64_t lef_square{ count_square({ -3, 0 }) };
    const int64_t rig_square{ count_square({ +3, 0 }) };

    const int64_t nne_square{ count_square({ +2, +2 }) };
    const int64_t nee_square{ count_square({ +2, +1 }) };
    const int64_t nnw_square{ count_square({ -2, +2 }) };
    const int64_t nww_square{ count_square({ -2, +1 }) };
    const int64_t sse_square{ count_square({ +2, -2 }) };
    const int64_t see_square{ count_square({ +2, -1 }) };
    const int64_t ssw_square{ count_square({ -2, -2 }) };
    const int64_t sww_square{ count_square({ -2, -1 }) };

    const int64_t k{ num_multiples_of_first_square };
    int64_t num_filled_odd_squares{ k * k };              // WolframAlpha math
    int64_t num_filled_even_squares{ (k - 1) * (k - 1) }; // WolframAlpha math
    int64_t num_parial_type_one{ k };                     // These go along the whole exposed edge
    int64_t num_parial_type_two{ k - 1 };                 // These are stopped by the corners

    int64_t num_final_positions{
        num_filled_odd_squares * filled_odd_square +
        num_filled_even_squares * filled_even_square +
        top_square + bot_square + lef_square + rig_square +
        num_parial_type_one * (nne_square + nnw_square + sse_square + ssw_square) +
        num_parial_type_two * (nee_square + nww_square + see_square + sww_square)
    };
    fmt::print("The result is: {}", num_final_positions);
    return num_final_positions != 238420328103151;
}
