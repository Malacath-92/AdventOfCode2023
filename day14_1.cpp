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

    std::vector panel{ file_data | lines | to_vec_chars | to_vector };

    using PanelRow = std::vector<char>;
    using Panel = std::vector<PanelRow>;
    enum class Direction
    {
        North,
        East,
        South,
        West,
    };
    static constexpr auto tilt = [](Panel& panel, Direction dir)
    {
        // Turn our array
        switch (dir)
        {
        case Direction::West:
            panel = transpose(panel);
            [[fallthrough]];
        case Direction::South:
            panel = transpose(panel);
            [[fallthrough]];
        case Direction::East:
            panel = transpose(panel);
            [[fallthrough]];
        case Direction::North:
            break;
        }

        // Do the thing
        for (size_t i = 0; i < panel.size(); i++)
        {
            PanelRow& row{ panel[i] };
            for (size_t j = 0; j < panel.size(); j++)
            {
                if (row[j] == '.')
                {
                    for (size_t k = i + 1; k < panel.size(); k++)
                    {
                        char& target{ panel[k][j] };
                        if (target == 'O')
                        {
                            std::swap(row[j], target);
                        }
                        else if (target == '#')
                        {
                            break;
                        }
                    }
                }
            }
        }

        // Turn back our array
        switch (dir)
        {
        case Direction::East:
            panel = transpose(panel);
            [[fallthrough]];
        case Direction::South:
            panel = transpose(panel);
            [[fallthrough]];
        case Direction::West:
            panel = transpose(panel);
            [[fallthrough]];
        case Direction::North:
            break;
        }
    };

    tilt(panel, Direction::North);

    size_t total_load{ 0 };
    for (size_t i = 0; i < panel.size(); i++)
    {
        PanelRow& row{ panel[i] };
        for (size_t j = 0; j < panel.size(); j++)
        {
            if (row[j] == 'O')
            {
                total_load += panel.size() - i;
            }
        }
    }

    fmt::print("The result is: {}", total_load);

    return total_load != 109654;
}
