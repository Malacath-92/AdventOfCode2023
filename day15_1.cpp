#include <cctype>
#include <ranges>
#include <string_view>

#include <fmt/format.h>

#include "algorithms.h"

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        fmt::print("Usage is exactly: *.exe input.txt");
        return 1;
    }

    static constexpr auto to_vector{ std::ranges::to<std::vector>() };
    static constexpr auto elements{ std::views::split(',') };
    static constexpr auto to_string_views{ std::views::transform(
        [](auto str)
        { return std::string_view(str.data(), str.size()); }) };

    const std::string_view input_file{ argv[1] };
    const std::string file_data{ algo::read_whole_file(input_file) };

    const std::vector instructions{ file_data | elements | to_string_views | to_vector };

    static constexpr auto hash = [](std::string_view str)
    {
        return algo::accumulate(
            str,
            [](uint8_t v, char c)
            { return (v + c) * 17; },
            uint8_t{ 0 });
    };
    const std::vector hashes{ instructions | std::views::transform(hash) | to_vector };

    const size_t sum_of_hashes{ algo::accumulate(hashes, size_t{ 0 }) };
    fmt::print("The result is: {}", sum_of_hashes);

    return sum_of_hashes != 521434;
}
