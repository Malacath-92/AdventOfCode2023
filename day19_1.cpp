#include <cctype>
#include <compare>
#include <queue>
#include <ranges>
#include <string_view>

#include <fmt/format.h>

#include "algorithms.h"
#include "tokenize.h"

enum class PartCategory
{
    ExtremelyCoolLooking,
    Musical,
    Aerodynamic,
    Shiny,
    Count,

    Cool = ExtremelyCoolLooking,
    Dynamic = Aerodynamic,
};

template<class T>
using CategoryArray = std::array<T, static_cast<size_t>(PartCategory::Count)>;

struct Part
{
    CategoryArray<int64_t> Ratings;
};

struct WorkflowRule
{
    PartCategory Category;
    bool LessThan; // otherwise greater than
    int64_t Rating;
    std::string_view NextWorkflow;
};

struct Workflow
{
    std::vector<WorkflowRule> Rules;
    std::string_view FinalRule;
};

struct WorkflowResult
{
    bool Accepted;
    Part AcceptedPart;
};

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        fmt::print("Usage is exactly: *.exe input.txt");
        return 1;
    }

    static constexpr auto to_vector{ std::ranges::to<std::vector>() };
    static constexpr auto to_unordered_map{ std::ranges::to<std::unordered_map>() };
    static constexpr auto lines{ std::views::split('\n') };
    static constexpr auto to_string_views{ std::views::transform(
        [](auto str)
        { return std::string_view(str.data(), str.size()); }) };

    static constexpr auto to_category = [](auto str)
    {
        using enum PartCategory;
        if (str == "x")
        {
            return Cool;
        }
        else if (str == "m")
        {
            return Musical;
        }
        else if (str == "a")
        {
            return Dynamic;
        }
        else
        {
            return Shiny;
        }
    };

    static constexpr auto to_workflow_rule = [](auto str)
    {
        const std::vector parts{ algo::split<"<>:", TokenizeBehavior::AnyOfDelimiter>(str) };
        return WorkflowRule{
            to_category(parts[0]),
            str[1] == '<',
            algo::stoi<int64_t>(parts[1]),
            parts[2],
        };
    };
    static constexpr auto to_workflows{ std::views::transform(
        [](auto str)
        {
            using namespace std::string_view_literals;
            auto name_and_rules{ str | std::views::split('{') | to_string_views | to_vector };
            const auto name{ name_and_rules[0] };
            std::vector rules{
                algo::trim(name_and_rules[1], '}') |
                std::views::split(',') |
                to_string_views |
                to_vector
            };
            auto last_rule{ rules.back() };
            rules.pop_back();
            auto first_rules{
                rules |
                std::views::transform(to_workflow_rule)
            };

            return std::pair<std::string_view, Workflow>{
                name,
                Workflow{
                    first_rules | to_vector,
                    last_rule,
                },
            };
        }) };
    static constexpr auto to_parts{ std::views::transform(
        [](auto str)
        {
            using namespace std::string_view_literals;
            auto categories{
                algo::trim(str, "{}"sv) |
                std::views::split(',') |
                to_string_views |
                std::views::transform(
                    [](auto str)
                    {
                        const std::vector cat_and_val{ str |
                                                       std::views::split('=') |
                                                       to_string_views |
                                                       to_vector };
                        return std::pair{ to_category(cat_and_val[0]), algo::stoi<int64_t>(cat_and_val[1]) };
                    })
            };

            Part part{};
            for (auto [category, rating] : categories)
            {
                part.Ratings[static_cast<size_t>(category)] = rating;
            }
            return part;
        }) };

    const std::string_view input_file{ argv[1] };
    const std::string file_data{ algo::read_whole_file(input_file) };

    const std::vector blocks{ algo::split<"\n\n">(file_data) };

    const std::unordered_map workflows{ blocks[0] | lines | to_string_views | to_workflows | to_unordered_map };
    const std::vector parts{ blocks[1] | lines | to_string_views | to_parts | to_vector };

    const auto funnel_through_workflows{
        std::views::transform(
            [&workflows](Part part)
            {
                std::string_view workflow_name{ "in" };
                while (workflow_name != "R" && workflow_name != "A")
                {
                    const auto this_workflow_name{ workflow_name };
                    const auto& workflow{ workflows.at(workflow_name) };
                    for (const auto& rule : workflow.Rules)
                    {
                        const auto i{ static_cast<size_t>(rule.Category) };
                        if ((rule.LessThan && part.Ratings[i] < rule.Rating) ||
                            (!rule.LessThan && part.Ratings[i] > rule.Rating))
                        {
                            workflow_name = rule.NextWorkflow;
                            break;
                        }
                    }

                    if (this_workflow_name == workflow_name)
                    {
                        workflow_name = workflow.FinalRule;
                    }
                }
                return WorkflowResult{
                    workflow_name == "A",
                    part,
                };
            }),
    };

    const std::vector accepted_parts{ parts | funnel_through_workflows | std::views::filter(&WorkflowResult::Accepted) | to_vector };

    static constexpr auto add_ratings = [](int64_t v, WorkflowResult res)
    {
        return v + algo::accumulate(res.AcceptedPart.Ratings, int64_t{ 0 });
    };

    const int64_t total_rating{ algo::accumulate(accepted_parts, add_ratings, int64_t{ 0 }) };
    fmt::print("The result is: {}", total_rating);
    return total_rating != 374873;
}
