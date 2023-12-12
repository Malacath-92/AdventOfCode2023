#include <string_view>

#include <cctype>
#include <ranges>

#include <fmt/format.h>

#include "algorithms.h"
#include "tokenize.h"

enum class SpringState
{
    Intact,
    Broken,
    Unknown,
};

struct BrokenSpringSegment
{
    SpringState State;
    int64_t Size;

    std::weak_ordering operator<=>(const BrokenSpringSegment&) const = default;
};

struct BrokenSpringMap
{
    std::string_view DirectMap;
    std::vector<int64_t> RangeMap;
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
        [](auto str)
        { return to_pair(str | std::views::split(' ') | to_string_views); }) };
    static constexpr auto chunk_blocks{ std::views::chunk_by([](char l, char r)
                                                             { return l == r; }) };
    static constexpr auto filter_broken{ std::views::filter([](auto str)
                                                            { return str.front() == '#'; }) };
    static constexpr auto sizes{ std::views::transform([](auto str)
                                                       { return static_cast<int64_t>(str.size()); }) };
    static constexpr auto to_numbers = [](auto str)
    {
        return str |
               std::views::split(',') |
               to_string_views |
               std::views::transform(&algo::stoi<int64_t>) |
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
        std::vector<BrokenSpringSegment> hypothetical_map;
        for (const auto& range : map_line.RangeMap)
        {
            hypothetical_map.push_back(BrokenSpringSegment{ SpringState::Broken, range });
        }
        for (size_t i = 0; i < map_line.DirectMap.size() - algo::accumulate(map_line.RangeMap, size_t{ 0 }); i++)
        {
            hypothetical_map.push_back(BrokenSpringSegment{ SpringState::Intact, 1 });
        }

        fmt::print("{}\n", map_line.DirectMap);

        // get first permuation
        algo::sort(hypothetical_map);

        static constexpr auto flatten = [](const auto& hypothetical_map, size_t out_size)
        {
            std::string flattened;
            flattened.reserve(out_size);
            for (const auto& range : hypothetical_map)
            {
                flattened.append(std::string(range.Size, state_to_char(range.State)));
            }
            return flattened;
        };

        // for each permuation
        while (std::next_permutation(hypothetical_map.begin(), hypothetical_map.end()))
        {
            static constexpr auto verify = [](const auto& expected, const auto& expected_sizes, const auto& hypothetical)
            {
                const auto chunks{ hypothetical | chunk_blocks | filter_broken | sizes | to_vector };
                if (chunks != expected_sizes)
                {
                    return false;
                }

                for (auto [e, h] : std::views::zip(expected, hypothetical))
                {
                    if (e != h && e != '?')
                    {
                        return false;
                    }
                }
                return true;
            };

            const auto flattened{ flatten(hypothetical_map, map_line.DirectMap.size()) };
            if (verify(map_line.DirectMap, map_line.RangeMap, flattened))
            {
                fmt::print("\t{}\n", flattened);
                number_configuration++;
            }
        }
    }

    fmt::print("The result is: {}", number_configuration);

    return number_configuration != 7718;
}
