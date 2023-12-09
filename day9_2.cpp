#include <string_view>

#include <cctype>
#include <ranges>
#include <unordered_map>

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

    static constexpr auto to_vector{ std::ranges::to<std::vector>() };
    static constexpr auto to_string_views{ std::views::transform(
        [](auto str)
        { return algo::trim(std::string_view(str.data(), str.size())); }) };
    static constexpr auto to_numbers{ std::views::transform(
        [](auto str)
        { return str | std::views::split(' ') | to_string_views | std::views::transform(&algo::stoi<int64_t>) | to_vector; }) };

    static constexpr auto difference{
        std::views::transform(
            [](const auto& two_numbers)
            {
                return std::get<1>(two_numbers) - std::get<0>(two_numbers);
            }),
    };
    static constexpr auto extrapolate{
        std::views::transform(
            [](this const auto& self, auto numbers) -> decltype(numbers)
            {
                static constexpr auto derivative{
                    [](const auto& numbers)
                    {
                        return numbers | std::views::adjacent<2> | difference | to_vector;
                    },
                };

                std::vector numbers_derivative{ derivative(numbers) };
                int64_t extrapolated_number{ numbers.front() };
                {
                    if (!algo::all_of(numbers_derivative, std::bind_back(std::equal_to<int64_t>{}, 0)))
                    {
                        const std::vector derivative_extrapolated{ self(std::move(numbers_derivative)) };
                        extrapolated_number = numbers.front() - derivative_extrapolated.front();
                    }
                }
                numbers.insert(numbers.begin(), extrapolated_number);
                return std::move(numbers);
            }),
    };
    static constexpr auto get_front{
        std::views::transform(
            [](const auto& numbers)
            {
                return numbers.front();
            }),
    };

    const std::string_view input_file{ argv[1] };
    const std::string file_data{ algo::read_whole_file(input_file) };

    const std::vector initial{ file_data | std::views::split('\n') | to_string_views | to_numbers | to_vector };
    const std::vector extrapolated{ initial | extrapolate | to_vector };
    const std::vector first_numbers{ extrapolated | get_front | to_vector };

    const int64_t sum_of_extrapolations{ algo::accumulate(first_numbers, int64_t{ 0 }) };
    fmt::print("The result is: {}", sum_of_extrapolations);

    return sum_of_extrapolations != 1062;
}
