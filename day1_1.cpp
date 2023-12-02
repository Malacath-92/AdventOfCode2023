#include <string_view>

#include <cctype>
#include <ranges>

#include <fmt/format.h>

#include "algorithms.h"
#include "tokenize.h"

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        fmt::print("Usage is exactly: *.exe input.txt");
        return 1;
    }

    const std::string_view input_file{ argv[1] };
    const std::string file_data{ algo::read_whole_file(input_file) };
    const std::vector lines{ algo::split<'\n'>(file_data) };

    struct Digits
    {
        char First;
        char Last;
    };
    auto line_to_digits = [](std::string_view lines)
    {
        static constexpr auto is_digit = [](char c)
        {
            return std::isdigit(c) != 0;
        };
        const char first{ *algo::find_if(lines, is_digit) };
        const char last{ *algo::find_if(std::ranges::reverse_view{ lines }, is_digit) };
        return Digits{ first, last };
    };
    const std::vector digits{
        algo::transformed<std::vector<Digits>>(
            lines, line_to_digits),
    };

    constexpr auto digits_to_number = [](Digits digits)
    {
        return 10 * static_cast<size_t>(digits.First - '0') + static_cast<size_t>(digits.Last - '0');
    };
    const std::vector numbers{
        algo::transformed<std::vector<size_t>>(
            digits, digits_to_number)
    };

    const size_t sum{ algo::accumulate(numbers, size_t{ 0 }) };
    fmt::print("The result is: {}", sum);

    return sum != 54597;
}
