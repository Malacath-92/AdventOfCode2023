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
    static constexpr auto is_gear_symbol = [](char c)
    {
        return c == '*';
    };
    static constexpr auto is_non_gear_symbol = [](char c)
    {
        return !is_gear_symbol(c) && c != ' ' && c != '\n' && !is_digit(c);
    };
    const std::string file_data{
        algo::replace(
            algo::replace(
                algo::read_whole_file(input_file),
                '.',
                ' '),
            is_non_gear_symbol,
            ' '),
    };
    const std::vector schematic{ algo::split<'\n', std::string>(file_data) };

    static constexpr auto get_adjacent_numbers = [](const std::vector<std::string>& schematic, size_t i, size_t j)
    {
        std::vector<size_t> numbers{};

        const size_t k_min{ static_cast<size_t>(algo::max(0, static_cast<int64_t>(i) - 1)) };
        const size_t k_max{ algo::min(schematic.size(), i + 2) };
        for (size_t k = k_min; k < k_max; k++)
        {
            const std::string& line{ schematic[k] };
            const size_t l_min{ static_cast<size_t>(algo::max(0, static_cast<int64_t>(j) - 1)) };
            const size_t l_max{ algo::min(line.size(), j + 2) };
            for (size_t l = l_min; l < l_max; l++)
            {
                if (is_digit(line[l]))
                {
                    while (l > 0 && is_digit(line[l - 1]))
                    {
                        --l;
                    }
                    const size_t left{ l };
                    while (l < line.size() && is_digit(line[l]))
                    {
                        ++l;
                    }
                    const size_t right{ l };
                    const std::string_view number_str{ line.data() + left, right - left };
                    numbers.push_back(ToType<size_t>(number_str));
                }
            }
        }
        return numbers;
    };

    struct Gear
    {
        size_t A;
        size_t B;
    };
    static constexpr auto get_two_value_gears = [](const std::vector<std::string>& schematic)
    {
        std::vector<Gear> gears{};
        for (size_t i = 0; i < schematic.size(); i++)
        {
            const std::string& line{ schematic[i] };
            for (size_t j = 0; j < line.size(); j++)
            {
                if (is_gear_symbol(line[j]))
                {
                    const std::vector numbers{ get_adjacent_numbers(schematic, i, j) };
                    if (numbers.size() == 2)
                    {
                        gears.push_back(Gear{ numbers[0], numbers[1] });
                    }
                }
            }
        }
        return gears;
    };
    const std::vector two_value_gears{ get_two_value_gears(schematic) };

    static constexpr auto get_gear_ratio = [](Gear gear) -> size_t
    {
        return gear.A * gear.B;
    };
    const auto gear_ratios{ algo::transformed(two_value_gears, get_gear_ratio) };

    const size_t sum_of_gears{ algo::accumulate(gear_ratios, size_t{ 0 }) };
    fmt::print("The result is: {}", sum_of_gears);

    return sum_of_gears != 76314915;
}
