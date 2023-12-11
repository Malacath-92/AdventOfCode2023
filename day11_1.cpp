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
    int64_t x;
    int64_t y;
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
    static constexpr auto filter_non_empty{ std::views::filter(
        [](auto i_and_str)
        { return !algo::contains_if(std::get<1>(i_and_str), [](char c)
                                    { return c != ' '; }); }) };
    static constexpr auto indices{ std::views::transform(
        [](auto i_and_str)
        { return std::get<0>(i_and_str); }) };

    const std::string_view input_file{ argv[1] };
    const std::string file_data{ algo::replace(algo::read_whole_file(input_file), '.', ' ') };

    const std::vector star_map{ file_data | lines | to_string_views | to_vector };

    const auto columns = [](const auto& arr)
    {
        return std::views::iota(size_t{ 0 }, arr.size()) |
               std::views::transform(
                   [&](auto i)
                   { return arr | std::views::transform([i](auto str)
                                                        { return str[i]; }) |
                            to_vector; }) |
               to_vector;
    };
    const std::vector star_map_transpose{ columns(star_map) };

    const auto empty_rows{ star_map | std::views::enumerate | filter_non_empty | indices | to_vector };
    const auto empty_columns{ star_map_transpose | std::views::enumerate | filter_non_empty | indices | std::views::reverse | to_vector };

    std::vector<std::string> star_map_expanded{};
    for (size_t i = 0; i < star_map.size(); i++)
    {
        std::string& row{ star_map_expanded.emplace_back(star_map[i]) };
        for (size_t j : empty_columns)
        {
            row.insert(row.begin() + j, ' ');
        }

        if (algo::contains(empty_rows, i))
        {
            star_map_expanded.push_back(row);
        }
    }

    std::vector<Position> stars{};
    for (size_t i = 0; i < star_map_expanded.size(); i++)
    {
        const std::string& row{ star_map_expanded[i] };
        for (size_t j = 0; j < row.size(); j++)
        {
            if (row[j] == '#')
            {
                stars.push_back({ static_cast<int64_t>(i), static_cast<int64_t>(j) });
            }
        }
    }

    std::vector<std::pair<Position, Position>> star_pairs{};
    for (size_t i = 0; i < stars.size(); i++)
    {
        for (size_t j = i + 1; j < stars.size(); j++)
        {
            star_pairs.push_back({ stars[i], stars[j] });
        }
    }

    static constexpr auto to_distances{ std::views::transform(
        [](const auto& star_pair)
        {
            const auto& [lhs, rhs] = star_pair;
            return std::abs(lhs.x - rhs.x) + std::abs(lhs.y - rhs.y);
        }) };
    const std::vector distances{ star_pairs | to_distances | to_vector };

    const size_t total_distances{ algo::accumulate(distances, size_t{ 0 }) };
    fmt::print("The result is: {}", total_distances);

    return total_distances != 9177603;
}
