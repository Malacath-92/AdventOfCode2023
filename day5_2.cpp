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

struct PropertyRange
{
    size_t Begin;
    size_t End;
};

struct PropertyMapping
{
    Property From;
    Property To;

    struct Range
    {
        size_t FromBegin;
        size_t FromEnd;
        size_t ToBegin;
    };
    std::vector<Range> Ranges;
};

struct Almanac
{
    std::vector<PropertyRange> RequiredSeeds;
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
        range.FromEnd = range.FromBegin + *(it++);
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

    const auto to_seed_range = std::views::transform(
        [](auto chunk)
        {
            const size_t from{ chunk[0] };
            const size_t size{ chunk[1] };
            return PropertyRange{ from, from + size };
        });
    return Almanac{
        raw_seeds_list | std::views::chunk(2) | to_seed_range | std::ranges::to<std::vector>(),
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

    const auto do_mapping = [&](std::vector<PropertyRange> value, Property from, Property to, auto& self) -> std::vector<PropertyRange>
    {
        while (std::to_underlying(to) - std::to_underlying(from) > 1)
        {
            const Property next{ static_cast<Property>(std::to_underlying(from) + 1) };
            value = self(std::move(value), from, next, self);
            from = next;
        }

        const PropertyMapping& mapping{ *algo::find(almanac.Mappings, &PropertyMapping::From, from) };

        std::vector<PropertyRange> out_ranges{};
        for (auto it = value.begin(); it != value.end();)
        {
            bool exhausted{ false };

            for (const auto& [from_begin, from_end, to_begin] : mapping.Ranges)
            {
                const auto& [input_begin, input_end] = *it;

                const size_t overlap_begin{
                    input_begin < from_begin
                        ? from_begin
                        : input_begin,
                };
                const size_t overlap_end{
                    input_end > from_end
                        ? from_end
                        : input_end,
                };
                if (overlap_begin < overlap_end)
                {
                    const size_t mapped_begin{ static_cast<size_t>(to_begin + (overlap_begin - from_begin)) };
                    const size_t mapped_end{ static_cast<size_t>(to_begin + (overlap_end - from_begin)) };
                    PropertyRange mapped_range{ mapped_begin, mapped_end };
                    out_ranges.push_back(mapped_range);

                    if (overlap_begin == input_begin && overlap_end == input_end)
                    {
                        it = value.erase(it);
                        exhausted = true;
                        break;
                    }
                    else if (overlap_begin == input_begin)
                    {
                        PropertyRange adjusted_range{ overlap_end, input_end };
                        *it = adjusted_range;
                    }
                    else if (overlap_end == input_end)
                    {
                        PropertyRange adjusted_range{ input_begin, overlap_begin };
                        *it = adjusted_range;
                    }
                    else
                    {
                        PropertyRange adjusted_range{ input_begin, overlap_begin };
                        *it = adjusted_range;

                        PropertyRange new_range{ overlap_end, input_end };
                        it = value.insert(it, new_range);
                    }
                }
            }

            if (!exhausted)
            {
                ++it;
            }
        }

        out_ranges.insert(out_ranges.end(), value.begin(), value.end());

        return out_ranges;
    };
    const auto map_seed_to_location = [&](PropertyRange value)
    {
        return do_mapping(std::vector{ value }, Property::Seed, Property::Location, do_mapping);
    };

    std::vector locations{ almanac.RequiredSeeds | std::views::transform(map_seed_to_location) | std::views::join | std::ranges::to<std::vector>() };
    std::vector lowest_locations{ locations | std::views::transform(&PropertyRange::Begin) | std::ranges::to<std::vector>() };

    const size_t lowest_location{ algo::min_element(lowest_locations) };
    fmt::print("The result is: {}", lowest_location);

    return lowest_location != 52210644;
}
