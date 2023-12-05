#include <string_view>

#include <cctype>
#include <ranges>

#include <fmt/format.h>
#include <magic_enum.hpp>

#include "algorithms.h"
#include "tokenize_to_types.h"

enum class Property
{
    Seed,
    Soil,
    Fertilizer,
    Water,
    Light,
    Temperature,
    Humidity,
    Location,
};

struct PropertyMapping
{
    Property From;
    Property To;

    struct Range
    {
        size_t FromBegin;
        size_t ToBegin;
        size_t Size;
    };
    std::vector<Range> Ranges;
};

struct Almanac
{
    std::vector<size_t> RequiredSeeds;
    std::vector<PropertyMapping> Mappings;
};

template<>
constexpr auto ToType<size_t>(std::string_view type_as_string)
{
    return algo::stoi<size_t>(type_as_string);
}

template<>
constexpr auto ToType<Property>(std::string_view type_as_string)
{
    return magic_enum::enum_cast<Property>(type_as_string, magic_enum::case_insensitive).value();
}

template<>
constexpr auto ToType<PropertyMapping::Range>(std::string_view type_as_string)
{
    PropertyMapping::Range range{};
    {
        const auto to_string_views{ std::views::transform([](auto sr)
                                                          { return std::string_view(sr.data(), sr.size()); }) };
        auto numbers{ std::views::split(type_as_string, ' ') | to_string_views | std::views::transform(&ToType<size_t>) };

        auto it = std::begin(numbers);
        range.ToBegin = *(it++);
        range.FromBegin = *(it++);
        range.Size = *(it++);
    }
    return range;
}

template<>
constexpr auto ToType<PropertyMapping>(std::string_view type_as_string)
{
    PropertyMapping mapping{};

    {
        const auto to_string_views{ std::views::transform([](auto sr)
                                                          { return std::string_view(sr.data(), sr.size()); }) };
        const auto is_property{ std::views::filter([](std::string_view str)
                                                   { return magic_enum::enum_contains<Property>(str, magic_enum::case_insensitive); }) };
        const auto to_property{ std::views::transform(&ToType<Property>) };

        const auto first_line{ *(std::views::split(type_as_string, '\n') | to_string_views).begin() };
        const auto mapping_specifier{ *(std::views::split(first_line, ' ') | to_string_views).begin() };
        auto properties{ std::views::split(mapping_specifier, '-') | to_string_views | is_property | to_property };

        auto it = std::begin(properties);
        mapping.From = *(it++);
        mapping.To = *(it++);

        const auto to_range{ std::views::transform(&ToType<PropertyMapping::Range>) };
        mapping.Ranges = std::views::split(type_as_string, '\n') | std::views::drop(1) | to_string_views | to_range | std::ranges::to<std::vector>();
    }

    return mapping;
}

template<>
constexpr auto ToType<Almanac>(std::string_view type_as_string)
{
    const std::vector paragraphs{ algo::split<"\n\n">(type_as_string) };
    const std::vector raw_seeds_list{ TokenizeToTypes<size_t>(algo::split<':'>(paragraphs[0])[1]) };

    const auto expand_range = std::views::transform(
        [](auto pair)
        {
            auto [from, size] = pair;
            return std::views::iota(*from, *from + *size) | std::ranges::to<std::vector>();
        });
    const auto a{
        raw_seeds_list | std::views::chunk(2) | expand_range | std::ranges::to<std::vector>()
    };
    //| std::views::join | std::ranges::to<std::vector>()
    return Almanac{
        {},
        // TokenizeToTypes<size_t>(algo::split<':'>(paragraphs[0])[1]) | std::views::chunk(2) | expand_range | flatten | std::ranges::to<std::vector>(),
        paragraphs | std::views::drop(1) | std::views::transform(&ToType<PropertyMapping>) | std::ranges::to<std::vector>(),
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

    const Almanac almanac{ ToType<Almanac>(file_data) };

    const auto do_mapping = [&](this auto self, size_t value, Property from, Property to) -> size_t
    {
        while (std::to_underlying(to) - std::to_underlying(from) > 1)
        {
            const Property next{ static_cast<Property>(std::to_underlying(from) + 1) };
            value = self(value, from, next);
            from = next;
        }

        const PropertyMapping& mapping{ *algo::find(almanac.Mappings, &PropertyMapping::From, from) };
        for (auto& [from_begin, to_begin, size] : mapping.Ranges)
        {
            if (value >= from_begin && value < from_begin + size)
            {
                return static_cast<size_t>(to_begin + (value - from_begin));
            }
        }
        return value;
    };
    const auto map_seed_to_location = [&](size_t value)
    {
        return do_mapping(value, Property::Seed, Property::Location);
    };

    std::vector locations{ almanac.RequiredSeeds | std::views::transform(map_seed_to_location) | std::ranges::to<std::vector>() };

    const size_t lowest_location{ algo::min_element(locations) };
    fmt::print("The result is: {}", lowest_location);

    return lowest_location != 340994526;
}
