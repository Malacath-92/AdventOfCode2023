#include <string_view>

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

    const std::string_view input_file{ argv[1] };
    const std::string file_data{ algo::read_whole_file(input_file) };

    const auto [directions, crossings_str]{ to_pair(algo::split<"\n\n">(file_data)) };
    const auto crossings{ crossings_str | std::views::split('\n') | to_crossing | to_umap };

    std::string_view at{ "AAA" };
    size_t num_turns{ 0 };
    do
    {
        const auto direction{ directions[num_turns % directions.size()] };
        const auto& crossing{ crossings.at(at) };
        at = direction == 'L' ? crossing.Left : crossing.Right;
        num_turns++;
    } while (at != "ZZZ");

    fmt::print("The result is: {}", num_turns);

    return num_turns != 19951;
}
