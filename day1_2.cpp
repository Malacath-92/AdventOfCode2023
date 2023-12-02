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

    using namespace std::string_view_literals;
    static constexpr std::array valid_digits{
        "0"sv,
        "1"sv,
        "one"sv,
        "2"sv,
        "two"sv,
        "3"sv,
        "three"sv,
        "4"sv,
        "four"sv,
        "5"sv,
        "five"sv,
        "6"sv,
        "six"sv,
        "7"sv,
        "seven"sv,
        "8"sv,
        "eight"sv,
        "9"sv,
        "nine"sv,
    };

    struct DigitStrs
    {
        std::string_view First;
        std::string_view Last;
    };
    auto line_to_digits = [](std::string_view lines)
    {
        static constexpr auto find_first_digit_str = [](std::string_view str)
        {
            for (size_t i = 0; i < str.size(); i++)
            {
                if (auto* digit_str{ algo::find_if(valid_digits, [str = str.substr(i)](std::string_view digit)
                                                   { return str.starts_with(digit); }) })
                {
                    return *digit_str;
                }
            }
            throw std::logic_error{
                "Invalid input..."
            };
        };
        const std::string_view first{ find_first_digit_str(lines) };

        static constexpr auto find_last_digit_str = [](std::string_view str)
        {
            for (size_t i = 0; i < str.size(); i++)
            {
                if (auto* digit_str{ algo::find_if(valid_digits, [str = str.substr(0, str.size() - i)](std::string_view digit)
                                                   { return str.ends_with(digit); }) })
                {
                    return *digit_str;
                }
            }
            throw std::logic_error{
                "Invalid input..."
            };
        };
        const std::string_view last{ find_last_digit_str(lines) };
        return DigitStrs{ first, last };
    };
    const std::vector digit_strs{
        algo::transformed<std::vector<DigitStrs>>(
            lines, line_to_digits),
    };

    struct Digits
    {
        size_t First;
        size_t Last;
    };
    constexpr auto digit_strs_to_digits = [](DigitStrs digits)
    {
        static constexpr auto digit_str_to_number = [](std::string_view digit_str)
        {
            if (digit_str == "0"sv)
            {
                return size_t{ 0 };
            }
            auto* it{ algo::find(valid_digits, digit_str) };
            return static_cast<size_t>((it - &valid_digits.front()) + 1) / 2;
        };
        return Digits{
            digit_str_to_number(digits.First),
            digit_str_to_number(digits.Last),
        };
    };
    const std::vector digits{
        algo::transformed<std::vector<Digits>>(
            digit_strs, digit_strs_to_digits),
    };

    constexpr auto digits_to_number = [](Digits digits)
    {
        return 10 * digits.First + digits.Last;
    };
    const std::vector numbers{
        algo::transformed<std::vector<size_t>>(
            digits, digits_to_number)
    };

    const size_t sum{ algo::accumulate(numbers, size_t{ 0 }) };
    fmt::print("The result is: {}", sum);

    return sum != 54504;
}
