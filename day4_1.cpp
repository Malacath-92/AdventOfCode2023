#include <string_view>

#include <cctype>
#include <ranges>

#include <fmt/format.h>

#include "algorithms.h"
#include "tokenize_to_types.h"

struct ScratchCard
{
    size_t Id;
    std::vector<size_t> Winning;
    std::vector<size_t> Have;
};

template<>
constexpr auto ToType<size_t>(std::string_view type_as_string)
{
    return algo::stoi<size_t>(type_as_string);
}

template<>
constexpr auto ToType<ScratchCard>(std::string_view type_as_string)
{
    std::vector parts{ algo::split<':'>(type_as_string) };
    const std::string_view card_id{ parts[0] };
    const std::vector numbers{ algo::split<'|'>(parts[1]) };
    return ScratchCard{
        ToType<size_t>(algo::split<' '>(card_id)[1]),
        TokenizeToTypes<size_t>(numbers[0]),
        TokenizeToTypes<size_t>(numbers[1]),
    };
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        fmt::print("Usage is exactly: *.exe input.txt");
        return 1;
    }

    const std::string_view input_file{ argv[1] };
    const std::string file_data{ algo::read_whole_file(input_file) };

    const std::vector cards{ TokenizeToTypes<ScratchCard, '\n'>(file_data) };

    static constexpr auto get_winning_numbers = [](const ScratchCard& card)
    {
        const auto is_winning_number = [&card](size_t number)
        { return algo::contains(card.Winning, number); };
        return card.Have | std::views::filter(is_winning_number) | std::ranges::to<std::vector>();
    };
    const std::vector winning_numbers{ algo::transformed(cards, get_winning_numbers) };

    static constexpr auto power_of_two = [](size_t i)
    { return static_cast<size_t>(std::pow(2, i)); };
    const size_t numbers_per_card{ cards.front().Have.size() };
    const std::vector powers_of_two{
        std::views::iota(size_t{ 0 }, numbers_per_card) |
            std::views::transform(power_of_two) |
            std::ranges::to<std::vector>(),
    };

    static const auto compute_points = [&](const auto& points)
    {
        return algo::accumulate(std::views::zip_transform(std::multiplies<>{}, points, powers_of_two), size_t{ 0 });
    };
    const std::vector winning_points{ algo::transformed(winning_numbers, compute_points) };

    const size_t sum_of_points{ algo::accumulate(winning_points, size_t{ 0 }) };
    fmt::print("The result is: {}", sum_of_points);

    return sum_of_points != 76314915;
}
