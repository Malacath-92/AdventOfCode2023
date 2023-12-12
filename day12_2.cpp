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
    std::string DirectMap;
    std::vector<size_t> RangeMap;
};

using MemoizationRangesContainer = std::span<const size_t>;
using MemoizationKey = std::pair<std::string, MemoizationRangesContainer>;

template<class T>
struct std::hash<std::span<const T>>
{
    std::size_t operator()(const std::span<const T>& s) const noexcept
    {
        size_t seed = 0;
        std::hash<T> hasher{};
        for (const T& i : s)
        {
            seed ^= hasher(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};
template<>
struct std::hash<MemoizationKey>
{
    std::size_t operator()(const MemoizationKey& m) const noexcept
    {
        size_t seed = 0;
        seed ^= std::hash<std::string>{}(m.first) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<MemoizationRangesContainer>{}(m.second) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

template<class T>
struct std::equal_to<std::span<T>>
{
    bool operator()(const std::span<T>& lhs, const std::span<T>& rhs) const noexcept
    {
        return lhs.size() == rhs.size() && std::memcmp(lhs.data(), rhs.data(), lhs.size_bytes()) == 0;
    }
};
template<>
struct std::equal_to<MemoizationKey>
{
    bool operator()(const MemoizationKey& lhs, const MemoizationKey& rhs) const noexcept
    {
        return std::equal_to<std::string>{}(lhs.first, rhs.first) &&
               std::equal_to<MemoizationRangesContainer>{}(lhs.second, rhs.second);
    }
};

using MemoizeMap = std::unordered_map<MemoizationKey, size_t>;

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
        {
            std::string direct_map{ pair_entry.first };
            std::vector raw_range_map{ to_numbers(pair_entry.second) };
            std::vector range_map{ raw_range_map };
            for (size_t i = 0; i < 4; i++)
            {
                direct_map += '?' + std::string{ pair_entry.first };
                range_map.insert(range_map.end(), raw_range_map.begin(), raw_range_map.end());
            }
            return BrokenSpringMap{
                std::move(direct_map),
                std::move(range_map),
            };
        }) };
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
        MemoizeMap memoize;

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

            MemoizationKey memo_key{ std::string{ map }, ranges };
            auto memo_it{ memoize.find(memo_key) };
            if (memo_it != memoize.end())
            {
                return memo_it->second;
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

            memoize[std::move(memo_key)] = result;
            return result;
        };

        const size_t result{ num_legal(map_line.DirectMap, map_line.RangeMap) };
        fmt::print("\t{}\n", result);
        number_configuration += result;
    }

    fmt::print("The result is: {}", number_configuration);

    return number_configuration != 128741994134728;
}
