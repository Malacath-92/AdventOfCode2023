#include <string_view>

#include <cctype>
#include <ranges>
#include <tuple>
#include <unordered_map>

#include <fmt/format.h>

#include "algorithms.h"
#include "tokenize.h"

enum class SpringState
{
    Intact,
    Broken,
    Unknown,
};

struct BrokenSpringMap
{
    std::string_view DirectMap;
    std::vector<size_t> RangeMap;
};

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        fmt::print("Usage is exactly: *.exe input.txt");
        return 1;
    }

    static constexpr auto to_vector{ std::ranges::to<std::vector>() };
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
    static constexpr auto lines{ std::views::split('\n') };
    static constexpr auto to_string_views{ std::views::transform(
        [](auto str)
        { return std::string_view(str.data(), str.size()); }) };
    static constexpr auto split_lines{ std::views::transform(
        [](const auto& str)
        { return to_pair(str | std::views::split(' ') | to_string_views); }) };
    static constexpr auto chunk_blocks{ std::views::chunk_by([](char l, char r)
                                                             { return l == r; }) };
    static constexpr auto filter_broken{ std::views::filter([](const auto& str)
                                                            { return str.front() == '#'; }) };
    static constexpr auto sizes{ std::views::transform([](const auto& str)
                                                       { return static_cast<size_t>(str.size()); }) };
    static constexpr auto to_numbers = [](const auto& str)
    {
        return str |
               std::views::split(',') |
               to_string_views |
               std::views::transform(&algo::stoi<size_t>) |
               to_vector;
    };
    static constexpr auto to_map{ std::views::transform(
        [](const auto& pair_entry)
        { return BrokenSpringMap{ pair_entry.first, to_numbers(pair_entry.second) }; }) };
    static constexpr auto state_to_char = [](auto state)
    {
        switch (state)
        {
        case SpringState::Intact:
            return '.';
        case SpringState::Broken:
            return '#';
        default:
            return '?';
        }
    };

    const std::string_view input_file{ argv[1] };
    const std::string file_data{ algo::read_whole_file(input_file) };

    const std::vector map{ file_data | lines | to_string_views | split_lines | to_map | to_vector };

    size_t number_configuration{ 0 };
    for (const auto& map_line : map)
    {
        fmt::print("{}\n", map_line.DirectMap);
        const auto num_legal = [&](this const auto& self,
                                   std::string_view map,
                                   std::span<const size_t> ranges) -> size_t
        {
            map = algo::trim(map, '.');

            if (map.empty())
            {
                return ranges.empty() ? 1 : 0;
            }

            if (ranges.empty())
            {
                return algo::contains(map, '#') ? 0 : 1;
            }

            size_t result{};
            if (map.starts_with('#'))
            {
                if (map.size() < ranges.front())
                {
                    result = 0; // string can't possibly fit the first range
                }
                else if (algo::contains(map.substr(0, ranges.front()), '.'))
                {
                    result = 0; // the part that should be all '#' or '?' contains '.'
                }
                // at this point the string is only '#' and '?'
                else if (map.size() == ranges.front())
                {
                    result = ranges.size() == 1 ? 1 : 0; // contains exactly the spring
                }
                else if (map[ranges.front()] == '#')
                {
                    result = 0; // the symbol after this hypothetical spring has to be '.' or '?'
                }
                else
                {
                    // ranges.front() + 1 to skip the separating '.'
                    result = self(map.substr(ranges.front() + 1), ranges.subspan(1));
                }
            }
            else if (map.starts_with('?'))
            {
                // replace for '?' once with '.' (omitted) and once with '#'
                result = self(map.substr(1), ranges) +
                         self('#' + std::string{ map.substr(1) }, ranges);
            }

            return result;
        };

        const size_t result{ num_legal(map_line.DirectMap, map_line.RangeMap) };
        fmt::print("\t{}\n", result);
        number_configuration += result;
    }

    fmt::print("The result is: {}", number_configuration);

    return number_configuration != 7718;
}
