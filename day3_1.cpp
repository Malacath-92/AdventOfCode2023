#include <string_view>

#include <cctype>
#include <ranges>

#include <fmt/format.h>

#include "algorithms.h"
#include "tokenize_to_types.h"

template<>
constexpr auto ToType<size_t>(std::string_view type_as_string)
{
    return algo::stoi<size_t>(type_as_string);
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        fmt::print("Usage is exactly: *.exe input.txt");
        return 1;
    }

    const std::string_view input_file{ argv[1] };

    static constexpr auto is_digit = [](char c)
    {
        return std::isdigit(c) != 0;
    };
    static constexpr auto is_symbol = [](char c)
    {
        return c != ' ' && c != '\n' && !is_digit(c);
    };
    const std::string file_data{
        algo::replace(
            algo::replace(
                algo::read_whole_file(input_file),
                '.',
                ' '),
            is_symbol,
            '*'),
    };
    const std::vector schematic{ algo::split<'\n', std::string>(file_data) };

    static constexpr auto is_adjacent_symbol = [](const std::vector<std::string>& schematic, size_t i, size_t j)
    {
        const size_t k_min{ static_cast<size_t>(algo::max(0, static_cast<int64_t>(i) - 1)) };
        const size_t k_max{ algo::min(schematic.size(), i + 2) };
        for (size_t k = k_min; k < k_max; k++)
        {
            const size_t l_min{ static_cast<size_t>(algo::max(0, static_cast<int64_t>(j) - 1)) };
            const size_t l_max{ algo::min(schematic[i].size(), j + 2) };
            for (size_t l = l_min; l < l_max; l++)
            {
                if (is_symbol(schematic[k][l]))
                {
                    return true;
                }
            }
        }
        return false;
    };
    static constexpr auto get_clean_schematics = [](const std::vector<std::string>& schematic)
    {
        std::vector<std::string> cleaned{ schematic };
        for (size_t i = 0; i < schematic.size(); i++)
        {
            const std::string& line{ schematic[i] };
            for (size_t j = 0; j < line.size(); j++)
            {
                if (is_digit(line[j]) && (j == 0 || !is_digit(line[j - 1])))
                {
                    const size_t initial_j{ j };
                    bool is_adjacent{ false };
                    while (is_digit(line[j]))
                    {
                        is_adjacent |= is_adjacent_symbol(schematic, i, j);
                        j++;
                    }
                    if (!is_adjacent)
                    {
                        for (size_t k = initial_j; k < j; k++)
                        {
                            cleaned[i][k] = ' ';
                        }
                    }
                }
            }
        }
        std::ranges::for_each(cleaned, [](auto& line)
                              { algo::replace(line, is_symbol, ' '); });
        return cleaned;
    };
    const std::vector cleaned{ get_clean_schematics(schematic) };
    const std::vector numbers{ TokenizeToTypes<size_t>(algo::join(cleaned, " ")) };

    const size_t sum_of_parts{ algo::accumulate(numbers, size_t{ 0 }) };
    fmt::print("The result is: {}", sum_of_parts);

    return sum_of_parts != 544433;
}
