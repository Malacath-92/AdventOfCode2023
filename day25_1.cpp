#include <cassert>
#include <cctype>
#include <compare>
#include <queue>
#include <ranges>
#include <string_view>

#include <fmt/format.h>

#include "algorithms.h"

struct Node
{
    std::string_view Name;
    std::string_view Connections;
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

    static constexpr auto to_nodes{ std::views::transform(
        [](auto str)
        {
            const std::vector parts{ str | std::views::split(':') | to_string_views | to_vector };
            return Node{ parts[0], parts[1] };
        }) };

    const std::string_view input_file{ argv[1] };
    const std::string file_data{ algo::read_whole_file(input_file) };

    const std::vector nodes_direct{ file_data | lines | to_string_views | to_nodes | to_vector };
    const std::vector nodes{
        [&]()
        {
            std::vector<Node> nodes{ nodes_direct };
            for (const Node& node : nodes_direct)
            {
                for (const auto& other_name : node.Connections | std::views::split(' ') | to_string_views)
                {
                    const auto* other{ algo::find(nodes, &Node::Name, other_name) };
                    if (other == nullptr)
                    {
                        nodes.push_back(Node{ other_name });
                    }
                }
            }
            return nodes;
        }()
    };

    const std::string digraph{
        [&]()
        {
            std::string str;
            for (const auto& lhs : nodes)
            {
                for (const auto& rhs_name : lhs.Connections | std::views::split(' ') | to_string_views)
                {
                    str += fmt::format("\t{} -> {}\n", lhs.Name, rhs_name);
                }
            }
            return fmt::format("digraph{{\n{}}}", str);
        }()
    };

    fmt::print("Inspect the digraph:\n{}\n", digraph);
    fmt::print("Enter the three connections to cut, separate the two nodes by space...\n");

    static constexpr auto read_connection_from_stdin = [](std::string_view label)
    {
        fmt::print("{}:", label);

        std::pair<std::string, std::string> conn{};
        std::cin >> conn.first;
        std::cin >> conn.second;
        return conn;
    };

    const auto [lhs_a, rhs_a] = read_connection_from_stdin(" First");
    const auto [lhs_b, rhs_b] = read_connection_from_stdin("Second");
    const auto [lhs_c, rhs_c] = read_connection_from_stdin(" Third");

    const std::unordered_map tree{
        [&]()
        {
            std::unordered_map<std::string_view, std::unordered_set<const Node*>> tree;
            for (const Node& lhs : nodes)
            {
                for (const auto& rhs_name : lhs.Connections | std::views::split(' ') | to_string_views)
                {
                    const auto should_cut = [&](const auto& lhs_cut, const auto& rhs_cut)
                    {
                        if ((lhs.Name == lhs_cut && rhs_name == rhs_cut) ||
                            (lhs.Name == rhs_cut && rhs_name == lhs_cut))
                        {
                            fmt::print("Cut connection {} <-> {}...\n", lhs_cut, rhs_cut);
                            return true;
                        }
                        return false;
                    };
                    if (should_cut(lhs_a, rhs_a) || should_cut(lhs_b, rhs_b) || should_cut(lhs_c, rhs_c))
                    {
                        continue;
                    }

                    const auto* rhs{ algo::find(nodes, &Node::Name, rhs_name) };
                    assert(rhs);
                    tree[rhs_name].insert(&lhs);
                    tree[lhs.Name].insert(rhs);
                }
            }
            return tree;
        }()
    };

    const size_t lhs_tree_size{
        [&]()
        {
            std::unordered_set<const Node*> visited;
            const auto visit_node = [&](this const auto& self, const Node* node)
            {
                if (visited.contains(node))
                {
                    return;
                }
                visited.insert(node);

                for (const Node* other : tree.at(node->Name))
                {
                    self(other);
                }
            };
            visit_node(&nodes[0]);
            return visited.size();
        }()
    };
    const size_t rhs_tree_size{ nodes.size() - lhs_tree_size };

    const size_t product_of_split_tree_sizes{ lhs_tree_size * rhs_tree_size };
    fmt::print("The result is: {}", product_of_split_tree_sizes);
    return product_of_split_tree_sizes != 495607;
}
