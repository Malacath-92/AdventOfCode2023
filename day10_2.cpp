#include <string_view>

#include <cctype>
#include <functional>
#include <ranges>
#include <unordered_map>

#include <Windows.h>
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

// too lazy to impl flood-fill myself
void flood_fill(std::vector<std::string>& map, int64_t x, int64_t y, char c)
{
    const int64_t max_x{ static_cast<int64_t>(map.front().size()) };
    const int64_t max_y{ static_cast<int64_t>(map.size()) };
    if (x < 0 || x >= max_x || y < 0 || y >= max_y)
    {
        return;
    }

    if (map[y][x] != ' ')
    {
        return;
    }

    map[y][x] = c;

    flood_fill(map, x + 1, y, c);
    flood_fill(map, x - 1, y, c);
    flood_fill(map, x, y + 1, c);
    flood_fill(map, x, y - 1, c);
}

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

    std::vector<std::string> map_cpy{};
    for (size_t i = 0; i < map.size(); i++)
    {
        map_cpy.push_back(std::string(map.front().size(), ' '));
    }
    for (const auto& pipe_elem : pipe)
    {
        // map_cpy[pipe_elem.pos.y][pipe_elem.pos.x] = 'x';
        map_cpy[pipe_elem.pos.y][pipe_elem.pos.x] = map[pipe_elem.pos.y][pipe_elem.pos.x];
    }

    while (pipe.front().pos != start_pos)
    {
        auto front{ pipe.front() };
        front.from = -front.from;
        pipe.push_back(front);
        pipe.erase(pipe.begin());
    }
    pipe.front().from.dx = static_cast<int64_t>(pipe.back().pos.x) - pipe.front().pos.x;
    pipe.front().from.dy = static_cast<int64_t>(pipe.back().pos.y) - pipe.front().pos.y;

    static constexpr auto rot_left = [](const auto& dir)
    {
        for (size_t i = 0; i < 4; i++)
        {
            if (directions[i] == dir)
            {
                return directions[(i + 1) % 4];
            }
        }

        throw std::logic_error{ "No connection..." };
    };
    for (auto& pipe_elem : pipe)
    {
        const auto back_dir{ pipe_elem.from };
        const auto out_dir{ rot_left(back_dir) };
        const auto out_fwd_dir{ Direction{ out_dir.dx - back_dir.dx, out_dir.dy - back_dir.dy } };
        const auto in_dir{ -out_dir };
        const auto in_fwd_dir{ Direction{ in_dir.dx - back_dir.dx, in_dir.dy - back_dir.dy } };

        const auto fill = [&](auto dir, char c)
        {
            const int64_t out_x{ static_cast<int64_t>(pipe_elem.pos.x) + dir.dx };
            const int64_t out_y{ static_cast<int64_t>(pipe_elem.pos.y) + dir.dy };
            flood_fill(map_cpy, out_x, out_y, c);
        };
        fill(out_dir, 'o');
        fill(in_dir, '+');
    }

    auto to_utf8 = [](const char* input)
    {
        wchar_t buffer[2048]{};
        MultiByteToWideChar(CP_UTF8, 0, input, -1, buffer, 2048);
        return std::wstring(buffer);
    };

    std::wstring map_u8;
    for (const auto& str : map_cpy)
    {
        map_u8 += to_utf8(str.data());
        map_u8 += L'\n';
    }
    algo::replace(map_u8, L'-', L'─');
    algo::replace(map_u8, L'|', L'│');
    algo::replace(map_u8, L'J', L'┘');
    algo::replace(map_u8, L'F', L'┌');
    algo::replace(map_u8, L'L', L'└');
    algo::replace(map_u8, L'7', L'┐');

    const auto num_enclosed{ algo::count(map_cpy | std::views::join, '+') };
    fmt::print("The result is: {}\n", num_enclosed);

    const auto num_outside{ algo::count(map_cpy | std::views::join, 'o') };
    fmt::print(" ... or maybe: {}", num_outside);

    return num_enclosed != 6931;
}
