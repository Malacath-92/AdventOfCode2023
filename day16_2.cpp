#include <cctype>
#include <ranges>
#include <string_view>

#include <fmt/format.h>

#include "algorithms.h"

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

    const std::vector contraption{ file_data | lines | to_string_views | to_vector };

    struct Vec2
    {
        int64_t X;
        int64_t Y;

        bool operator==(const Vec2&) const = default;
        Vec2 operator+(const Vec2& rhs) const
        {
            return Vec2{ X + rhs.X, Y + rhs.Y };
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
    struct Beam
    {
        Vec2 Position;
        Vec2 Direction;
    };

    const auto get_energized = [&](Beam starting_beam)
    {
        std::vector energized(contraption.size(), std::vector(contraption.front().size(), ' '));
        std::vector analyzed(contraption.size(), std::vector(contraption.front().size(), std::vector<Vec2>{}));

        std::vector<Beam> beams{ starting_beam };
        while (!beams.empty())
        {
            for (auto it = beams.begin(); it != beams.end();)
            {
                {
                    Beam& beam{ *it };

                    const auto& [x, y] = beam.Position;
                    const auto& [dx, dy] = beam.Direction;

                    energized[y][x] = '0';

                    const char& tile{ contraption[y][x] };
                    switch (tile)
                    {
                    case '/':
                        beam.Direction = dx != 0
                                             ? beam.Direction.rot_left()
                                             : beam.Direction.rot_right();
                        break;
                    case '\\':
                        beam.Direction = dy != 0
                                             ? beam.Direction.rot_left()
                                             : beam.Direction.rot_right();
                        break;
                    case '|':
                        if (dx != 0)
                        {
                            Beam split_beam{ beam.Position, beam.Direction.rot_right() };
                            beam.Direction = beam.Direction.rot_left();
                            it = beams.insert(it, split_beam);
                        }
                        break;
                    case '-':
                        if (dy != 0)
                        {
                            Beam split_beam{ beam.Position, beam.Direction.rot_right() };
                            beam.Direction = beam.Direction.rot_left();
                            it = beams.insert(it, split_beam);
                        }
                        break;
                    }
                }

                Beam& beam{ *it };
                beam.Position = beam.Position + beam.Direction;

                const auto& [x, y] = beam.Position;
                if (x < 0 ||
                    y < 0 ||
                    static_cast<size_t>(x) >= contraption.size() ||
                    static_cast<size_t>(y) >= contraption.front().size() ||
                    algo::contains(analyzed[y][x], beam.Direction))
                {
                    it = beams.erase(it);
                }
                else
                {
                    analyzed[y][x].push_back(beam.Direction);
                    ++it;
                }
            }
        }

        const size_t num_energized{ algo::accumulate(
            energized, [](size_t v, const auto& row)
            { return v + algo::count(row, '0'); },
            size_t{ 0 }) };
        return num_energized;
    };

    const std::vector starting_beams{
        [&]()
        {
            const int64_t max_x{ static_cast<int64_t>(contraption.size()) - 1 };
            const int64_t max_y{ static_cast<int64_t>(contraption.front().size()) - 1 };

            std::vector<Beam> beams;
            for (int64_t x = 0; x <= max_x; x++)
            {
                beams.push_back({ { x, 0 }, { 0, 1 } });
                beams.push_back({ { x, max_y }, { 0, -1 } });
            }
            for (int64_t y = 0; y <= max_y; y++)
            {
                beams.push_back({ { 0, y }, { 1, 0 } });
                beams.push_back({ { max_x, y }, { -1, 0 } });
            }
            return beams;
        }()
    };

    const std::vector energy_levels{ starting_beams |
                                     std::views::transform(get_energized) |
                                     to_vector };

    const size_t num_energized{ algo::max_element(energy_levels) };
    fmt::print("The result is: {}", num_energized);

    return num_energized != 8061;
}
