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
        { return static_cast<int64_t>(std::get<0>(i_and_str)); }) };

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
    const auto empty_cols{ star_map_transpose | std::views::enumerate | filter_non_empty | indices | std::views::reverse | to_vector };

    std::vector<Position> stars{};
    for (size_t i = 0; i < star_map.size(); i++)
    {
        const std::string_view& row{ star_map[i] };
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

    const auto to_distances{ std::views::transform(
        [&](const auto& star_pair)
        {
            const auto& [lhs, rhs] = star_pair;
            const auto lhs_x{ std::min(lhs.x, rhs.x) };
            const auto lhs_y{ std::min(lhs.y, rhs.y) };
            const auto rhs_x{ std::max(lhs.x, rhs.x) };
            const auto rhs_y{ std::max(lhs.y, rhs.y) };
            const auto empty_rows_within{ empty_rows | std::views::filter([&](int64_t row)
                                                                          { return row > lhs_x && row < rhs_x; }) |
                                          to_vector };
            const auto empty_cols_within{ empty_cols | std::views::filter([&](int64_t row)
                                                                          { return row > lhs_y && row < rhs_y; }) |
                                          to_vector };
            const auto n_rows{ static_cast<int64_t>(empty_rows_within.size()) };
            const auto n_cols{ static_cast<int64_t>(empty_cols_within.size()) };

            static constexpr int64_t expansion_rate{ 2 };
            return (rhs_x - lhs_x + rhs_y - lhs_y) + n_rows * (expansion_rate - 1) + n_cols * (expansion_rate - 1);
        }) };
    const std::vector distances{ star_pairs | to_distances | to_vector };

    const size_t total_distances{ algo::accumulate(distances, size_t{ 0 }) };
    fmt::print("The result is: {}", total_distances);

    return total_distances != 9177603;
}
