#include <string_view>

#include <cassert>
#include <cctype>
#include <ranges>
#include <unordered_map>

#include <fmt/format.h>

#include "algorithms.h"
#include "tokenize.h"

struct Map
{
    std::string_view Directions;

    struct Crossing
    {
        std::string_view Left;
        std::string_view Right;
    };
    std::unordered_map<std::string_view, Crossing> Crossings;
};

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        fmt::print("Usage is exactly: *.exe input.txt");
        return 1;
    }

    static constexpr auto to_string_views{ std::views::transform(
        [](auto str)
        { return algo::trim(std::string_view(str.data(), str.size())); }) };
    static constexpr auto to_vector{ std::ranges::to<std::vector>() };
    static constexpr auto to_umap{ std::ranges::to<std::unordered_map>() };

    static constexpr auto to_pair{
        [](auto range)
        {
            auto it{ range.begin() };
            const auto& first{ *it };
            ++it;
            const auto& second{ *it };
            return std::pair{ first, second };
        },
    };
    static constexpr auto to_crossing{
        std::views::transform(
            [](auto str)
            {
                auto [starting, towards_str]{ to_pair(str | std::views::split('=') | to_string_views) };
                auto is_bracket = [](char c)
                {
                    using namespace std::string_view_literals;
                    return "()"sv.contains(c);
                };
                auto [left, right]{ to_pair(algo::trim(towards_str, is_bracket) | std::views::split(',') | to_string_views) };
                return std::pair{ starting, Map::Crossing{ left, right } };
            }),
    };

    static constexpr auto to_start{ std::views::transform([](const auto& entry)
                                                          { return entry.first; }) };
    static constexpr auto to_ghost_start{ std::views::filter([](const auto& start)
                                                             { return start.ends_with('A'); }) };

    const std::string_view input_file{ argv[1] };
    const std::string file_data{ algo::read_whole_file(input_file) };

    const auto [directions, crossings_str]{ to_pair(algo::split<"\n\n">(file_data)) };
    const auto crossings{ crossings_str | std::views::split('\n') | to_crossing | to_umap };

    std::vector<size_t> cycle_lengths{};
    {
        std::vector all_at{ crossings | to_start | to_ghost_start | to_vector };
        size_t num_turns{ 0 };
        while (!all_at.empty())
        {
            const auto direction{ directions[num_turns % directions.size()] };
            num_turns++;
            for (auto it = all_at.begin(); it != all_at.end();)
            {
                auto& at{ *it };
                const auto& crossing{ crossings.at(at) };
                at = direction == 'L' ? crossing.Left : crossing.Right;
                if (at.ends_with('Z'))
                {
                    cycle_lengths.push_back(num_turns);
                    it = all_at.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
    }

    size_t least_common_multiple{ cycle_lengths[0] };
    for (size_t num_turns : cycle_lengths | std::views::drop(1))
    {
        least_common_multiple = std::lcm(least_common_multiple, num_turns);
    }
    fmt::print("The result is: {}", least_common_multiple);

    return least_common_multiple != 16342438708751;
}
