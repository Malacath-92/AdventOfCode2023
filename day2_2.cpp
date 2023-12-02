#include <string_view>

#include <cctype>
#include <ranges>

#include <fmt/format.h>

#include "algorithms.h"
#include "tokenize_to_types.h"

struct HandSample
{
    std::string_view Color{};
    size_t Amount{ 0 };
};

struct Hand
{
    size_t Green{ 0 };
    size_t Blue{ 0 };
    size_t Red{ 0 };
};

struct Bag : Hand
{
};

struct Game
{
    size_t Id{ 0 };
    std::vector<Hand> Hands{};
};

struct GameLowerBound
{
    size_t Id{ 0 };
    Hand LowerBoundHand{};
};

template<>
constexpr auto ToType<size_t>(std::string_view type_as_string)
{
    return algo::stoi<size_t>(type_as_string);
}

template<>
constexpr auto ToType<HandSample>(std::string_view type_as_string)
{
    std::vector parts{ algo::split<' '>(type_as_string) };
    return HandSample{ parts[1], ToType<size_t>(parts[0]) };
}

template<>
constexpr auto ToType<Hand>(std::string_view type_as_string)
{
    const auto samples{ TokenizeToTypes<HandSample, ','>(type_as_string) };

    Hand hand{};
    for (const auto& [color, amount] : samples)
    {
        if (color == "green")
        {
            hand.Green = amount;
        }
        else if (color == "blue")
        {
            hand.Blue = amount;
        }
        else if (color == "red")
        {
            hand.Red = amount;
        }
    }
    return hand;
}

template<>
constexpr auto ToType<Game>(std::string_view type_as_string)
{
    const auto intro_and_game{ algo::split<':'>(type_as_string) };

    const auto intro{ intro_and_game[0] };
    const size_t id{ ToType<size_t>(algo::split<' '>(intro)[1]) };

    const auto game{ intro_and_game[1] };
    std::vector hands{ TokenizeToTypes<Hand, ';'>(game) };

    return Game{ id, std::move(hands) };
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

    const std::vector games{ TokenizeToTypes<Game, '\n'>(file_data) };
    const std::vector lower_bounds{
        algo::transformed<std::vector<GameLowerBound>>(
            games,
            [](const Game& game)
            {
                size_t max_green{ algo::max_index(game.Hands, &Hand::Green) };
                size_t max_blue{ algo::max_index(game.Hands, &Hand::Blue) };
                size_t max_red{ algo::max_index(game.Hands, &Hand::Red) };
                return GameLowerBound{
                    game.Id,
                    Hand{
                        game.Hands[max_green].Green,
                        game.Hands[max_blue].Blue,
                        game.Hands[max_red].Red,
                    },
                };
            }),
    };

    constexpr auto power = [](const GameLowerBound& game)
    {
        return game.LowerBoundHand.Green * game.LowerBoundHand.Blue * game.LowerBoundHand.Red;
    };
    auto powers{ lower_bounds | std::views::transform(power) };

    const auto sum_of_powers{ algo::accumulate(powers, size_t{ 0 }) };
    fmt::print("The result is: {}", sum_of_powers);

    return sum_of_powers != 78669;
}
