#include <string_view>

#include <cctype>
#include <ranges>

#include <fmt/format.h>
#include <magic_enum.hpp>

#include "algorithms.h"
#include "tokenize_to_types.h"

struct RaceData
{
    size_t Time;
    size_t Record;
};
struct RaceSolution
{
    size_t Lower;
    size_t Upper;
};

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

    static constexpr auto to_string_views{ std::views::transform([](auto str)
                                                                 { return std::string_view(str.data(), str.size()); }) };
    static constexpr auto filter_empty{ std::views::filter([](auto str)
                                                           { return !str.empty(); }) };
    static constexpr auto to_size_t{ std::views::transform(&ToType<size_t>) };
    static constexpr auto to_race_data{ std::views::transform([](auto pair)
                                                              { return RaceData{ std::get<0>(pair), std::get<1>(pair) }; }) };
    static constexpr auto to_lines{ std::views::split('\n') };
    static constexpr auto to_vector{ std::ranges::to<std::vector>() };
    static constexpr auto drop_start{
        std::views::transform(
            [](auto str)
            {
                const size_t colon{ str.find(':') + 1 };
                return str.substr(colon);
            }),
    };
    static constexpr auto split_to_numbers{
        std::views::transform(
            [](auto str)
            {
                return str | std::views::split(' ') | to_string_views | filter_empty | to_size_t | to_vector;
            }),
    };

    const std::string_view input_file{ argv[1] };
    const std::string file_data{ algo::read_whole_file(input_file) };

    const std::vector raw_numbers{ file_data | to_lines | to_string_views | drop_start | split_to_numbers | to_vector };
    const std::vector races{ std::views::zip(raw_numbers[0], raw_numbers[1]) | to_race_data | to_vector };

    static constexpr auto get_solutions{
        std::views::transform(
            [](RaceData race) -> RaceSolution
            {
                const size_t T_race{ race.Time };
                const size_t L_record{ race.Record };

                const double root_term{ std::sqrt(static_cast<double>(T_race * T_race - 4 * L_record)) };
                const double lower_bound{ (T_race - root_term) / 2.0 };
                const double upper_bound{ (T_race + root_term) / 2.0 };

                // Solves T * (T_race - T) > L_record for T
                return RaceSolution{
                    static_cast<size_t>(std::floor(lower_bound) + 1),
                    static_cast<size_t>(std::ceil(upper_bound) - 1),
                };
            }),
    };
    const std::vector solutions{ races | get_solutions | to_vector };

    static constexpr auto count_solutions{ std::views::transform([](auto sol)
                                                                 { return sol.Upper - sol.Lower + 1; }) };
    const std::vector number_solutions{ solutions | count_solutions | to_vector };

    const size_t product_of_num_solutions{ algo::accumulate(number_solutions, std::multiplies<>{}, size_t{ 1 }) };
    fmt::print("The result is: {}", product_of_num_solutions);

    return product_of_num_solutions != 781200;
}
