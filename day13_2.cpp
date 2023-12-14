#include <string_view>

#include <cctype>
#include <ranges>

#include <fmt/format.h>

#include "algorithms.h"
#include "tokenize.h"

inline constexpr bool c_DebugOutput{ false };

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        fmt::print("Usage is exactly: *.exe input.txt");
        return 1;
    }

    static constexpr auto to_vector{ std::ranges::to<std::vector>() };
    static constexpr auto lines{ std::views::split('\n') };
    static constexpr auto to_string_views{ std::views::transform(
        [](auto str)
        { return std::string_view(str.data(), str.size()); }) };
    static constexpr auto to_vec_chars{ std::views::transform(
        [](auto str)
        { return std::vector<char>(str.begin(), str.end()); }) };

    static constexpr auto transpose = [](const auto& vec)
    {
        const size_t num_rows{ vec.front().size() };
        return std::views::iota(size_t{ 0 }, num_rows) |
               std::views::transform(
                   [&](auto i)
                   { return vec | std::views::transform([i](const auto& inner_vec)
                                                        { return inner_vec[i]; }) |
                            to_vector; }) |
               to_vector;
    };

    const std::string_view input_file{ argv[1] };
    const std::string file_data{ algo::read_whole_file(input_file) };

    const std::vector surface{ algo::split<"\n\n">(file_data) |
                               std::views::transform([](std::string_view str)
                                                     { return str |
                                                              lines |
                                                              to_vec_chars |
                                                              to_vector; }) |
                               to_vector };

    std::vector<size_t> rows{};
    std::vector<size_t> cols{};
    for (const auto& vec : surface)
    {
        std::vector<std::vector<char>> found_row_a{};
        std::vector<std::vector<char>> found_row_b{};
        std::vector<std::vector<char>> found_col_a{};
        std::vector<std::vector<char>> found_col_b{};

        static constexpr auto equal = [](const auto& top, const auto& bot)
        {
            bool has_error{ false };
            for (auto [trow, brow] : std::views::zip(top, bot))
            {
                for (auto [tc, bc] : std::views::zip(trow, brow))
                {
                    if (tc != bc)
                    {
                        if (has_error)
                        {
                            return false;
                        }
                        else
                        {
                            has_error = true;
                        }
                    }
                }
            }
            return has_error;
        };

        static constexpr auto get_top_bot = [](const auto& vec, size_t i)
        {
            std::vector top{ vec | std::views::take(i) | to_vector };
            std::vector bot{ vec | std::views::drop(i) | to_vector };

            if (top.size() > bot.size())
            {
                top = top | std::views::drop(top.size() - bot.size()) | to_vector;
            }
            else if (bot.size() > top.size())
            {
                bot = bot | std::views::take(top.size()) | to_vector;
            }

            bot = bot | std::views::reverse | to_vector;

            return std::pair{ std::move(top), std::move(bot) };
        };

        for (size_t i = 0; i < vec.size(); i++)
        {
            const auto [top, bot] = get_top_bot(vec, i);
            if (equal(top, bot))
            {
                rows.push_back(i);
                if constexpr (c_DebugOutput)
                {
                    found_row_a = top;
                    found_row_b = bot | std::views::reverse | to_vector;
                }
            }
        }

        const auto vect{ transpose(vec) };
        for (size_t i = 0; i < vect.size(); i++)
        {
            const auto [top, bot] = get_top_bot(vect, i);
            if (equal(top, bot))
            {
                cols.push_back(i);
                if constexpr (c_DebugOutput)
                {
                    found_col_a = top;
                    found_col_b = bot | std::views::reverse | to_vector;
                }
            }
        }

        if constexpr (c_DebugOutput)
        {
            if (!found_row_a.empty())
            {
                for (size_t i = 0; i < vec.size(); i++)
                {
                    const std::string_view orig_row{ vec[i].data(), vec[i].size() };

                    const int64_t j{ static_cast<int64_t>(i) - static_cast<int64_t>(rows.back() - found_row_a.size()) };
                    const std::string_view found_mirr_a{ j >= 0 && static_cast<int64_t>(found_row_a.size()) > j
                                                             ? std::string_view{ found_row_a[j].data(), found_row_a[j].size() }
                                                             : std::string_view{ "" } };

                    const int64_t k{ static_cast<int64_t>(i) - static_cast<int64_t>(rows.back()) };
                    const std::string_view found_mirr_b{ k >= 0 && static_cast<int64_t>(found_row_b.size()) > k
                                                             ? std::string_view{ found_row_b[k].data(), found_row_b[k].size() }
                                                             : std::string_view{ "" } };

                    const bool print_note{ j == static_cast<int64_t>(found_row_a.size()) - 1 || k == 0 };
                    const std::string note{ print_note
                                                ? fmt::format("<< {}", i)
                                                : "" };

                    fmt::print("{}   {}{}{}\n", orig_row, found_mirr_a, found_mirr_b, note);
                }
                fmt::print("\n");
            }
            else if (!found_col_a.empty())
            {
                for (size_t i = 0; i < vect.size(); i++)
                {
                    const std::string_view orig_col{ vect[i].data(), vect[i].size() };

                    const int64_t j{ static_cast<int64_t>(i) - static_cast<int64_t>(cols.back() - found_col_a.size()) };
                    const std::string_view found_mirr_a{ j >= 0 && static_cast<int64_t>(found_col_a.size()) > j
                                                             ? std::string_view{ found_col_a[j].data(), found_col_a[j].size() }
                                                             : std::string_view{ "" } };

                    const int64_t k{ static_cast<int64_t>(i) - static_cast<int64_t>(cols.back()) };
                    const std::string_view found_mirr_b{ k >= 0 && static_cast<int64_t>(found_col_b.size()) > k
                                                             ? std::string_view{ found_col_b[k].data(), found_col_b[k].size() }
                                                             : std::string_view{ "" } };

                    const bool print_note{ j == static_cast<int64_t>(found_col_a.size()) - 1 || k == 0 };
                    const std::string note{ print_note
                                                ? fmt::format("<< {}", i)
                                                : "" };

                    fmt::print("{}   {}{}{}\n", orig_col, found_mirr_a, found_mirr_b, note);
                }
                fmt::print("\n");
            }
            else
            {
                fmt::print("\nFound no reflection!!!\n");
                for (size_t i = 0; i < vec.size(); i++)
                {
                    const std::string_view orig_row{ vec[i].data(), vec[i].size() };
                    fmt::print("{}\n", orig_row);
                }
                fmt::print("\n");
                for (size_t i = 0; i < vect.size(); i++)
                {
                    const std::string_view orig_col{ vect[i].data(), vect[i].size() };
                    fmt::print("{}\n", orig_col);
                }
                fmt::print("\n");
            }
        }
    }

    const size_t sum_of_notes{ algo::accumulate(cols, size_t{ 0 }) + 100 * algo::accumulate(rows, size_t{ 0 }) };
    fmt::print("The result is: {}", sum_of_notes);

    return sum_of_notes != 32312;
}
