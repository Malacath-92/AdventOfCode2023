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

struct EvaluatedScratchCard
{
    size_t Id;
    size_t AmountOfWinningNumbers;
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
        ToType<size_t>(algo::split<' '>(card_id).back()),
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
    static constexpr auto evaluate_card = [](const ScratchCard& card)
    {
        return EvaluatedScratchCard{ card.Id, get_winning_numbers(card).size() };
    };
    const std::vector evaluated_cards{ algo::transformed(cards, evaluate_card) };

    std::vector amount_of_cards_per_id{ size_t{ 1 } };
    const auto get_amount_of_times_won = [&evaluated_cards, &amount_of_cards_per_id](const EvaluatedScratchCard& card)
    {
        const auto is_won_by_card = [&card](const EvaluatedScratchCard& lower_card)
        {
            return card.Id - lower_card.Id <= lower_card.AmountOfWinningNumbers;
        };
        const auto get_precumputed_number_of_times_won = [&amount_of_cards_per_id](const EvaluatedScratchCard& lower_card)
        {
            return amount_of_cards_per_id[lower_card.Id - 1];
        };
        return 1 + algo::accumulate(evaluated_cards |
                                        std::views::take(card.Id - 1) |
                                        std::views::filter(is_won_by_card) |
                                        std::views::transform(get_precumputed_number_of_times_won) |
                                        std::ranges::to<std::vector>(),
                                    size_t{ 0 });
    };
    for (size_t i = 1; i < evaluated_cards.size(); i++)
    {
        amount_of_cards_per_id.push_back(get_amount_of_times_won(evaluated_cards[i]));
    }

    const size_t sum_of_points{ algo::accumulate(amount_of_cards_per_id, size_t{ 0 }) };
    fmt::print("The result is: {}", sum_of_points);

    return sum_of_points != 5571760;
}
