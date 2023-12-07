#include <string_view>

#include <array>
#include <cctype>
#include <compare>
#include <ranges>

#include <fmt/format.h>
#include <magic_enum.hpp>

#include "algorithms.h"

inline constexpr std::string_view c_CardRanking{ " 23456789TQKA" };

enum class HandType
{
    HighCard,
    OnePair,
    TwoPair,
    ThreeOfAKind,
    FullHouse,
    FourOfAKind,
    FiveOfAKind,
};

struct ReducedHand
{
    HandType Type;
    std::array<size_t, 5> Indices;

    friend std::weak_ordering operator<=>(const ReducedHand&, const ReducedHand&) = default;
};

struct Hand
{
    std::string_view Cards;
    size_t Bid;
    ReducedHand Reduced;
};

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        fmt::print("Usage is exactly: *.exe input.txt");
        return 1;
    }

    static constexpr auto to_string_views{ std::views::transform([](auto str)
                                                                 { return std::string_view(str.data(), str.size()); }) };
    static constexpr auto to_vector{ std::ranges::to<std::vector>() };

    static constexpr auto get_reduced_hand{
        [](const auto& hand)
        {
            static constexpr auto get_type = [](const auto& cards)
            {
                const auto num_joker{ algo::count(cards, 'J') };
                static constexpr auto is_n_of_a_kind = [](const auto& cards, std::ptrdiff_t N)
                {
                    return algo::any_of(c_CardRanking, [&](char c)
                                        { return algo::count(cards, c) == N; });
                };
                const auto is_n_of_a_kind_with_joker = [num_joker](const auto& cards, std::ptrdiff_t N)
                {
                    return algo::any_of(c_CardRanking, [&, N, num_joker](char c)
                                        { return algo::count(cards, c) + num_joker >= N; });
                };
                const auto is_full_house = [num_joker](const auto& cards)
                {
                    // num_joker >= 3 always evaluates to something better
                    if (num_joker == 2)
                    {
                        // single pair without joker
                        return is_n_of_a_kind(cards, 2);
                    }
                    else if (num_joker == 1)
                    {
                        // two pair without joker
                        if (const char* kind{ algo::find_if(c_CardRanking, [&](char c)
                                                            { return algo::count(cards, c) == 2; }) })
                        {
                            return algo::any_of(c_CardRanking, [&](char c)
                                                { return c != *kind && algo::count(cards, c) == 2; });
                        }
                        else
                        {
                            return false;
                        }
                    }
                    else
                    {
                        return is_n_of_a_kind(cards, 2) && is_n_of_a_kind(cards, 3);
                    }
                };
                const auto is_two_pair = [num_joker](const auto& cards)
                {
                    // num_joker >= 2 always evaluates to something better
                    if (num_joker == 1)
                    {
                        return is_n_of_a_kind(cards, 2);
                    }
                    else if (const char* kind{ algo::find_if(c_CardRanking, [&](char c)
                                                             { return algo::count(cards, c) == 2; }) })
                    {
                        return algo::any_of(c_CardRanking, [&](char c)
                                            { return c != *kind && algo::count(cards, c) == 2; });
                    }
                    return false;
                };

                using enum HandType;
                if (is_n_of_a_kind_with_joker(cards, 5))
                {
                    return FiveOfAKind;
                }
                else if (is_n_of_a_kind_with_joker(cards, 4))
                {
                    return FourOfAKind;
                }
                else if (is_full_house(cards))
                {
                    return FullHouse;
                }
                else if (is_n_of_a_kind_with_joker(cards, 3))
                {
                    return ThreeOfAKind;
                }
                else if (is_two_pair(cards))
                {
                    return TwoPair;
                }
                else if (is_n_of_a_kind_with_joker(cards, 2))
                {
                    return OnePair;
                }
                else
                {
                    return HighCard;
                }
            };

            static constexpr auto get_card_index = [](char c)
            {
                for (size_t i = 0; i < c_CardRanking.size(); i++)
                {
                    if (c == c_CardRanking[i])
                    {
                        return i;
                    }
                }
                return size_t{ 0 };
            };

            ReducedHand reduced_hand{ get_type(hand.Cards) };
            std::transform(hand.Cards.begin(), hand.Cards.end(), reduced_hand.Indices.begin(), get_card_index);
            return reduced_hand;
        },
    };
    static constexpr auto to_hand{
        std::views::transform(
            [](const auto& str)
            {
                auto parts{ str | std::views::split(' ') | to_string_views };
                auto it{ parts.begin() };
                Hand hand{ { *it }, algo::stoi<size_t>(*(++it)) };
                hand.Reduced = get_reduced_hand(hand);
                return hand;
            }),
    };

    static constexpr auto to_points{
        std::views::transform(
            [](const auto& i_and_hand)
            {
                const auto& [i, hand] = i_and_hand;
                return (i + 1) * hand.Bid;
            }),
    };

    const std::string_view input_file{ argv[1] };
    const std::string file_data{ algo::read_whole_file(input_file) };

    auto hands{ file_data | std::views::split('\n') | to_string_views | to_hand | to_vector };
    std::ranges::sort(hands, [](const Hand& lhs, const Hand& rhs)
                      { return lhs.Reduced < rhs.Reduced; });

    auto points{ hands | std::views::enumerate | to_points | to_vector };

    const size_t sum_of_points{ algo::accumulate(points, size_t{ 0 }) };
    fmt::print("The result is: {}", sum_of_points);

    return sum_of_points != 253362743;
}
