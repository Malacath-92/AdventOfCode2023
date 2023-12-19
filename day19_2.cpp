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

struct PartRange
{
    int64_t From{ 1 };
    int64_t To{ 4000 };
};

struct RangedPart
{
    CategoryArray<PartRange> Ratings;
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

struct WorkflowProcess
{
    RangedPart Part;
    std::string_view Workflow;
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

    const std::string_view input_file{ argv[1] };
    const std::string file_data{ algo::read_whole_file(input_file) };

    const std::unordered_map workflows{ algo::split<"\n\n">(file_data)[0] | lines | to_string_views | to_workflows | to_unordered_map };

    static constexpr auto count_range_combinations = [](RangedPart part)
    {
        return (part.Ratings[0].To - part.Ratings[0].From + 1) *
               (part.Ratings[1].To - part.Ratings[1].From + 1) *
               (part.Ratings[2].To - part.Ratings[2].From + 1) *
               (part.Ratings[3].To - part.Ratings[3].From + 1);
    };
    const auto count_combinations = [&workflows](this const auto& self, WorkflowProcess process) -> int64_t
    {
        if (process.Workflow == "A")
        {
            return count_range_combinations(process.Part);
        }
        else if (process.Workflow == "R")
        {
            return 0;
        }
        else
        {
            size_t sum_of_all{ 0 };

            const auto& part{ process.Part };
            const auto& workflow{ workflows.at(process.Workflow) };
            for (const auto& rule : workflow.Rules)
            {
                const auto i{ static_cast<size_t>(rule.Category) };
                if ((rule.LessThan && part.Ratings[i].To < rule.Rating) ||
                    (!rule.LessThan && part.Ratings[i].From > rule.Rating))
                {
                    return self(WorkflowProcess{ part, rule.NextWorkflow });
                }
                else
                {
                    // Failed a rule, split into two ranges ...
                    WorkflowProcess success{ process };
                    success.Workflow = rule.NextWorkflow;
                    WorkflowProcess failure{ process };

                    if (rule.LessThan)
                    {
                        success.Part.Ratings[i].To = std::min(success.Part.Ratings[i].To, rule.Rating - 1);
                        failure.Part.Ratings[i].From = std::max(success.Part.Ratings[i].From, rule.Rating);
                    }
                    else
                    {
                        failure.Part.Ratings[i].To = std::min(success.Part.Ratings[i].To, rule.Rating);
                        success.Part.Ratings[i].From = std::max(success.Part.Ratings[i].From, rule.Rating + 1);
                    }

                    sum_of_all += self(success);
                    process = failure;
                }
            }

            return sum_of_all + self(WorkflowProcess{ part, workflow.FinalRule });
        }
    };

    const int64_t num_accepted_combinations{ count_combinations(WorkflowProcess{ RangedPart{}, "in" }) };
    fmt::print("The result is: {}", num_accepted_combinations);
    return num_accepted_combinations != 122112157518711;
}
