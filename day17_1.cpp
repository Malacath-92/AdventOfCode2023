#include <cctype>
#include <compare>
#include <queue>
#include <ranges>
#include <string_view>

#include <fmt/format.h>
#include <fmt/ranges.h>

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
    static constexpr auto char_to_int = [](char c)
    {
        return c - '0';
    };

    const std::string_view input_file{ argv[1] };
    const std::string file_data{ algo::read_whole_file(input_file) };

    const std::vector city{ file_data | lines | to_string_views | to_vector };

    struct Vec2
    {
        int64_t X;
        int64_t Y;

        auto operator<=>(const Vec2&) const = default;

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
    struct Payload
    {
        size_t HeatLoss;
        Vec2 Position;
        Vec2 Direction;
        size_t NumForward;
        std::vector<Vec2> Path;

        auto operator<=>(const Payload&) const = default;
    };

    const Vec2 city_min{ 0, 0 };
    const Vec2 city_max{ static_cast<int64_t>(city.front().size()), static_cast<int64_t>(city.size()) };
    const auto out_of_bounds = [&](const auto& coord)
    {
        const auto& [x, y] = coord;
        return x < city_min.X ||
               y < city_min.Y ||
               x >= city_max.X ||
               y >= city_max.Y;
    };

    const auto print_path = [&](const auto& path)
    {
        size_t heat_loss{ 0 };
        std::string path_str{};
        for (int64_t x = 0; x < city_max.X; x++)
        {
            for (int64_t y = 0; y < city_max.Y; y++)
            {
                if (algo::contains(path, Vec2{ x, y }))
                {
                    const char num{ city[x][y] };
                    path_str += num;
                    heat_loss += char_to_int(num);
                }
                else
                {
                    path_str += '.';
                }
            }
            path_str += '\n';
        }
        heat_loss -= char_to_int(city[0][0]);
        fmt::print("{}", path_str);
        fmt::print("Heat Loss: {}\n\n", heat_loss);
    };

    struct Visited
    {
        Vec2 Direction;
        size_t NumForward;

        std::weak_ordering operator<=>(const Visited&) const = default;
    };
    std::vector visited(city_max.X, std::vector(city_max.Y, std::set<Visited>{}));

    std::priority_queue<Payload, std::vector<Payload>, std::greater<Payload>> payloads{};
    payloads.push({ 0, { 0, 0 }, { 1, 0 }, 0 });
    payloads.push({ 0, { 0, 0 }, { 0, 1 }, 0 });

    std::optional<size_t> minimum_heatloss{ std::nullopt };
    std::optional<std::vector<Vec2>> minimum_path{ std::nullopt };
    while (!payloads.empty())
    {
        const Payload payload{ payloads.top() };
        payloads.pop();

        const auto& [x, y] = payload.Position;

        {
            Visited v{ payload.Direction, payload.NumForward };
            if (visited[x][y].contains(v))
            {
                continue;
            }
            visited[x][y].insert(v);
        }

        const bool on_target{
            x == city_max.X - 1 &&
            y == city_max.Y - 1
        };
        if (on_target)
        {
            if (!minimum_heatloss.has_value() || payload.HeatLoss < minimum_heatloss.value())
            {
                minimum_heatloss = payload.HeatLoss;
                minimum_path = payload.Path;
            }
            continue;
        }

        const auto push = [&](Vec2 p, Vec2 d, size_t h, size_t n, std::vector<Vec2> pp)
        {
            p = p + d;
            if (out_of_bounds(p))
            {
                return;
            }

            pp.push_back(p);
            h += char_to_int(city[p.X][p.Y]);
            payloads.push(Payload{ h, p, d, n, std::move(pp) });
        };

        if (payload.NumForward < 3)
        {
            push(payload.Position, payload.Direction, payload.HeatLoss, payload.NumForward + 1, payload.Path);
        }

        push(payload.Position, payload.Direction.rot_left(), payload.HeatLoss, 1, payload.Path);
        push(payload.Position, payload.Direction.rot_right(), payload.HeatLoss, 1, payload.Path);
    }

    if (minimum_path.has_value())
    {
        print_path(minimum_path.value());
    }

    fmt::print("The result is: {}", minimum_heatloss.value_or(0));

    return minimum_heatloss.value() != 7728;
}
