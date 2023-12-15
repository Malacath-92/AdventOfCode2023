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

    const std::vector raw_instructions{ file_data | elements | to_string_views | to_vector };

    static constexpr auto hash = [](std::string_view str)
    {
        return algo::accumulate(
            str,
            [](uint8_t v, char c)
            { return (v + c) * 17; },
            uint8_t{ 0 });
    };

    enum class Operation : uint8_t
    {
        Remove,
        Add,
    };
    struct Instruction
    {
        std::string_view Label;
        uint8_t LabelHash;
        Operation Op;
        uint8_t FocalLength;
    };
    static constexpr auto to_instruction = [](std::string_view str)
    {
        if (str.ends_with('-'))
        {
            const std::string_view label{ str.substr(0, str.size() - 1) };
            return Instruction{
                label,
                hash(label),
                Operation::Remove,
            };
        }
        else
        {
            const std::vector parts{ str | std::views::split('=') | to_string_views | to_vector };
            return Instruction{
                parts[0],
                hash(parts[0]),
                Operation::Add,
                algo::stoi<uint8_t>(parts[1]),
            };
        }
    };

    const std::vector instructions{ raw_instructions | std::views::transform(to_instruction) | to_vector };

    struct Lens
    {
        std::string_view Label;
        uint8_t FocalLength;
    };

    struct Box
    {
        std::vector<Lens> Lenses;
    };
    std::array<Box, 256> boxes{};

    for (const auto& inst : instructions)
    {
        Box& box{ boxes[inst.LabelHash] };
        if (inst.Op == Operation::Remove)
        {
            algo::erase(box.Lenses, &Lens::Label, inst.Label);
        }
        else if (Lens* lens{ algo::find(box.Lenses, &Lens::Label, inst.Label) })
        {
            lens->FocalLength = inst.FocalLength;
        }
        else
        {
            box.Lenses.push_back(Lens{ inst.Label, inst.FocalLength });
        }
    }

    size_t total_lens_power{ 0 };
    for (const auto& [i, box] : std::views::enumerate(boxes))
    {
        for (const auto& [j, lens] : std::views::enumerate(box.Lenses))
        {
            total_lens_power += (i + 1) * (j + 1) * lens.FocalLength;
        }
    }

    fmt::print("The result is: {}", total_lens_power);
    return total_lens_power != 521434;
}
