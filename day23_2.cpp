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

    Vec2 operator+(const Vec2& rhs) const
    {
        return Vec2{ X + rhs.X, Y + rhs.Y };
    }
    Vec2 operator-(const Vec2& rhs) const
    {
        return Vec2{ X - rhs.X, Y - rhs.Y };
    }
};

struct Node;

struct Edge
{
    size_t Length;
    size_t To;
};

struct Node
{
    Vec2 Pos;
    size_t Id;
    std::vector<Edge> Edges;
};

struct Hike
{
    size_t Length;
    Node* AtNode;
    std::unordered_set<Node*> Visited;

    auto operator<=>(const Hike& rhs) const
    {
        return Length <=> rhs.Length;
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

    const std::vector landscape{ file_data | lines | to_string_views | to_vector };

    const Vec2 landscape_min{ 0, 0 };
    const Vec2 landscape_max{ static_cast<int64_t>(landscape.front().size()), static_cast<int64_t>(landscape.size()) };
    const auto out_of_bounds = [&](const auto& coord)
    {
        const auto& [x, y] = coord;
        if (x < landscape_min.X ||
            y < landscape_min.Y ||
            x >= landscape_max.X ||
            y >= landscape_max.Y)
        {
            return true;
        }

        const auto tile{ landscape[y][x] };
        return tile == '#';
    };

    std::vector<Node> nodes{
        Node{ { 1, 0 }, 0 },
        Node{ { landscape_max.X - 2, landscape_max.Y - 1 }, 1 },
    };

    {
        std::vector visited(landscape_max.X, std::vector(landscape_max.Y, false));
        visited[0][1] = true;

        std::vector<std::tuple<size_t, Vec2, Vec2>> nexts{
            { nodes[0].Id, nodes[0].Pos, nodes[0].Pos + Vec2{ 0, 1 } },
            { nodes[1].Id, nodes[1].Pos, nodes[1].Pos - Vec2{ 0, 1 } },
        };
        while (!nexts.empty())
        {
            const auto [i, from, start] = nexts.front();
            nexts.erase(nexts.begin());

            {
                const auto& [x, y] = start;
                if (visited[y][x])
                {
                    continue;
                }
                visited[y][x] = true;
            }

            size_t length{ 1 };
            Vec2 prev{ from };
            Vec2 to{ start };
            while (true)
            {
                const std::array dirs{
                    to + Vec2{ +1, 0 },
                    to + Vec2{ -1, 0 },
                    to + Vec2{ 0, +1 },
                    to + Vec2{ 0, -1 },
                };

                if (algo::count(dirs, out_of_bounds, true) < 2)
                {
                    break;
                }

                for (auto& next : dirs)
                {
                    if (!out_of_bounds(next) && next != prev)
                    {
                        prev = to;
                        to = next;
                        length++;
                        break;
                    }
                }

                {
                    const auto& [x, y] = to;
                    if (visited[y][x])
                    {
                        break;
                    }
                    visited[y][x] = true;
                }
            }

            if (Node * node{ algo::find(nodes, &Node::Pos, to) })
            {
                nodes[i].Edges.push_back({ length, static_cast<size_t>(node - &nodes.front()) });
                node->Edges.push_back({ length, i });
            }
            else
            {
                const auto try_push = [&](Vec2 p)
                {
                    const auto& [x, y] = p;
                    if (out_of_bounds(p))
                    {
                        return;
                    }
                    else if (visited[y][x] && !algo::contains(nodes, &Node::Pos, p))
                    {
                        return;
                    }
                    nexts.push_back({ nodes.size(), to, p });
                };
                try_push(to + Vec2{ +1, 0 });
                try_push(to + Vec2{ -1, 0 });
                try_push(to + Vec2{ 0, +1 });
                try_push(to + Vec2{ 0, -1 });

                nodes[i].Edges.push_back({ length, nodes.size() });
                nodes.push_back(Node{ to, nodes.size(), { { length, i } } });
            }
        }
    }

    std::optional<size_t> maximum_path{ std::nullopt };
    std::vector<Hike> hikes{
        Hike{ 0, &nodes[0] }
    };

    while (!hikes.empty())
    {
        const Hike hike{ hikes.back() };
        hikes.pop_back();

        for (const Edge& edge : hike.AtNode->Edges)
        {
            const auto [len, to] = edge;
            if (hike.Visited.contains(&nodes[to]))
            {
                continue;
            }

            {
                const auto [tx, ty] = nodes[to].Pos;
                const bool on_target{
                    tx == landscape_max.X - 2 &&
                    ty == landscape_max.Y - 1
                };
                if (on_target)
                {
                    const size_t total_length{ hike.Length + edge.Length };
                    if (!maximum_path.has_value() || total_length > maximum_path.value())
                    {
                        fmt::print("Found new max path: {}\n", total_length);
                        maximum_path = total_length;
                    }
                    continue;
                }
            }

            Hike next_hike{ hike };
            next_hike.Length += edge.Length;
            next_hike.AtNode = &nodes[to];
            next_hike.Visited.insert(hike.AtNode);
            hikes.push_back(next_hike);
        }
    }

    if (!maximum_path.has_value())
    {
        fmt::print("No path found...");
        return 1;
    }

    const size_t maximum_path_length{ maximum_path.value() };
    fmt::print("The result is: {}", maximum_path_length);
    return maximum_path_length != 6450;
}
