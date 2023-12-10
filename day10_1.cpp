#include <string_view>

#include <cctype>
#include <functional>
#include <ranges>
#include <unordered_map>

#include <fmt/format.h>

#include "algorithms.h"
#include "tokenize.h"

struct Position
{
    size_t x;
    size_t y;
    bool operator==(const Position&) const = default;
};
struct Direction
{
    int64_t dx;
    int64_t dy;

    Direction operator-() const
    {
        return Direction{ -dx, -dy };
    }
    bool operator==(const Direction&) const = default;
};

struct PipeRecord
{
    Position pos;
    Direction from;
    size_t distance;
};

namespace std
{
template<>
struct hash<Position>
{
    size_t operator()(const Position& pos) const
    {
        auto hash_x{ hash<size_t>{}(pos.x) };
        auto hash_y{ hash<size_t>{}(pos.y) };

        if (hash_x != hash_y)
        {
            return hash_x ^ hash_y;
        }
        return hash_x;
    }
};
template<>
struct hash<Direction>
{
    size_t operator()(const Direction& dir) const
    {
        auto hash_x{ hash<int64_t>{}(dir.dx) };
        auto hash_y{ hash<int64_t>{}(dir.dy) };

        if (hash_x != hash_y)
        {
            return hash_x ^ hash_y;
        }
        return hash_x;
    }
};
} // namespace std

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

    static constexpr std::array directions{
        Direction{ 1, 0 },
        Direction{ 0, 1 },
        Direction{ -1, 0 },
        Direction{ 0, -1 },
    };

    using ConnectingPieces = std::array<char, 3>;
    static const std::unordered_map<Direction, ConnectingPieces> connections{
        { directions[0], ConnectingPieces{ '-', 'J', '7' } },
        { directions[1], ConnectingPieces{ '|', 'J', 'L' } },
        { directions[2], ConnectingPieces{ '-', 'F', 'L' } },
        { directions[3], ConnectingPieces{ '|', 'F', '7' } },
    };

    static constexpr auto next_direction = [](const auto& map, Position pos, Direction from_dir, bool ignore_from_cons = false)
    {
        const auto& from{ map[pos.y][pos.x] };
        for (const auto& dir : directions)
        {
            if (dir == from_dir)
            {
                continue;
            }

            const int64_t to_x{ static_cast<int64_t>(pos.x) + dir.dx };
            const int64_t to_y{ static_cast<int64_t>(pos.y) + dir.dy };
            const int64_t max_x{ static_cast<int64_t>(map.front().size()) };
            const int64_t max_y{ static_cast<int64_t>(map.size()) };
            if (to_x >= 0 && to_x < max_x && to_y >= 0 && to_y < max_y)
            {
                const auto& to{ map[to_y][to_x] };
                const auto& cons_to{ connections.at(dir) };
                const auto& cons_from{ connections.at(-dir) };
                if (algo::contains(cons_to, to) && (ignore_from_cons || algo::contains(cons_from, from)))
                {
                    return dir;
                }
            }
        }

        throw std::logic_error{ "No connection..." };
    };
    static constexpr auto next_record = [](const auto& map, PipeRecord rec)
    {
        const auto dir{ next_direction(map, rec.pos, rec.from) };
        const size_t to_x{ rec.pos.x + dir.dx };
        const size_t to_y{ rec.pos.y + dir.dy };
        return PipeRecord{ { to_x, to_y }, -dir, rec.distance + 1 };
    };

    const std::string_view input_file{ argv[1] };
    const std::string file_data{ algo::read_whole_file(input_file) };

    const std::vector map{ file_data | lines | to_string_views | to_vector };

    const size_t start_y{ static_cast<size_t>(algo::find(map, [](const auto& line)
                                                         { return algo::contains(line, 'S'); }) -
                                              &map.front()) };
    const size_t start_x{ map[start_y].find('S') };
    const Position start_pos{ start_x, start_y };
    const auto first_direction{ next_direction(map, start_pos, directions[0], true) };
    const auto second_direction{ next_direction(map, start_pos, first_direction, true) };
    const Position first_pos{ start_pos.x + first_direction.dx, start_pos.y + first_direction.dy };
    const Position second_pos{ start_pos.x + second_direction.dx, start_pos.y + second_direction.dy };

    std::vector<PipeRecord> pipe{
        { first_pos, -first_direction, 1 },
        { start_pos, {}, 0 },
        { second_pos, -second_direction, 1 },
    };

    while (true)
    {
        {
            const auto& front{ pipe.front() };
            const auto next{ next_record(map, front) };
            if (algo::contains(pipe, &PipeRecord::pos, next.pos))
            {
                break;
            }
            else
            {
                pipe.insert(pipe.begin(), next);
            }
        }

        {
            const auto& back{ pipe.back() };
            const auto next{ next_record(map, back) };
            if (algo::contains(pipe, &PipeRecord::pos, next.pos))
            {
                break;
            }
            else
            {
                pipe.insert(pipe.end(), next);
            }
        }
    }

    const size_t furthest_distance{ algo::max_element(pipe | std::views::transform(&PipeRecord::distance)) };
    fmt::print("The result is: {}", furthest_distance);

    return furthest_distance != 6931;
}
