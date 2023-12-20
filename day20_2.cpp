#include <cctype>
#include <compare>
#include <queue>
#include <ranges>
#include <string_view>

#include <fmt/format.h>

#include "algorithms.h"
#include "tokenize.h"

enum class Signal
{
    Low,
    High,
};

enum class ModuleType
{
    Input,
    Output,
    FlipFlop,
    Conjunction,
};

enum class ModuleState : uint8_t
{
    On,
    Off,
};

struct Module;
struct PendingSignal;
struct FullModuleState;

using ModuleList = std::vector<std::string_view>;
using SignalList = std::vector<Signal>;
using ModuleStateList = std::vector<FullModuleState>;
using ModuleMap = std::unordered_map<std::string_view, Module>;
using PendingSignalList = std::vector<PendingSignal>;

struct Module
{
    std::string_view Name;
    ModuleType Type;
    ModuleList Outputs;

    // For Conjunction
    ModuleList Inputs;
    SignalList LastSignals;

    // For FlipFlop
    ModuleState State{ ModuleState::Off };
};

struct PendingSignal
{
    std::string_view FromName;
    std::string_view ToName;
    Signal Sig;
};

struct FullModuleState
{
    ModuleState State;
    SignalList LastSignals;

    bool operator==(const FullModuleState&) const = default;
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

    static constexpr auto to_modules{
        std::views::transform(
            [](auto str)
            {
                using enum ModuleType;

                const std::vector parts{ algo::split<"->", TokenizeBehavior::TrimWhitespace>(str) };

                std::string_view name{ parts[0] };
                ModuleType type{ name == "output" ? Output : Input };
                if (name.starts_with('%') || name.starts_with('&'))
                {
                    type = name.starts_with('%') ? FlipFlop : Conjunction;
                    name = name.substr(1);
                }

                std::vector outputs{ algo::split<",", TokenizeBehavior::TrimWhitespace>(parts[1]) };
                return std::pair{
                    name,
                    Module{
                        name,
                        type,
                        std::move(outputs),
                    },
                };
            }),
    };
    static constexpr auto to_module_state =
        [](const auto& mod)
    {
        using enum ModuleType;
        switch (mod.Type)
        {
        case Input:
            [[fallthrough]];
        case Output:
            return ModuleState::On;
        case FlipFlop:
            return mod.State;
        case Conjunction:
            [[fallthrough]];
        default:
            return algo::all_of(mod.LastSignals, [](auto sig)
                                { return sig == Signal::High; })
                       ? ModuleState::On
                       : ModuleState::Off;
        };
    };
    static constexpr auto to_full_module_state = [](const auto& name_mod)
    {
        return FullModuleState{
            to_module_state(name_mod.second),
            name_mod.second.LastSignals,
        };
    };

    const std::string_view input_file{ argv[1] };
    const std::string file_data{ algo::read_whole_file(input_file) };

    ModuleMap modules{ file_data | lines | to_string_views | to_modules | to_unordered_map };
    for (auto& [name, mod] : modules)
    {
        for (auto out : mod.Outputs)
        {
            auto it{ modules.find(out) };
            if (it != modules.end())
            {
                auto& out_mod{ it->second };
                if (out_mod.Type == ModuleType::Conjunction)
                {
                    out_mod.Inputs.push_back(name);
                    out_mod.LastSignals.push_back(Signal::Low);
                }
            }
        }
    }

    // We have 4 sub-trees, starting at xk, cn, rj, gf 
    //                     if we ignore rz, mr, kv, jg respectively
    struct SubTree
    {
        std::string_view Root;
        std::string_view Output;
    };
    const std::array subtrees{
        SubTree{ "xk", "rz" },
        SubTree{ "cn", "mr" },
        SubTree{ "rj", "kv" },
        SubTree{ "gf", "jg" },
    };

    struct SubtreeLoops
    {
        size_t Start;
        size_t Length;
    };
    std::vector<SubtreeLoops> loops{};
    for (const auto& [root, output] : subtrees)
    {
        ModuleMap subtree{};
        auto build_subtree = [&](this const auto& self, auto mod_name)
        {
            if (subtree.contains(mod_name))
            {
                return;
            }

            auto it{ modules.find(mod_name) };
            if (it != modules.end())
            {
                auto& mod{ it->second };
                subtree[mod_name] = mod;
                if (mod_name != output)
                {
                    for (auto& out : mod.Outputs)
                    {
                        self(out);
                    }
                }
            }
        };
        build_subtree(root);

        std::vector<ModuleStateList> seen_states;
        size_t cycle_start{ 0 };
        size_t cycle_len{ 0 };
        while (true)
        {
            ModuleStateList states{ subtree | std::views::transform(to_full_module_state) | to_vector };
            if (const ModuleStateList * old{ algo::find(seen_states, states) })
            {
                cycle_start = static_cast<size_t>(old - &seen_states.front());
                cycle_len = static_cast<size_t>(seen_states.size() - cycle_start);
                break;
            }
            seen_states.push_back(states);

            PendingSignalList pending_signals{ { "broadcaster", root, Signal::Low } };
            while (!pending_signals.empty())
            {
                PendingSignalList next_signals{};
                for (auto [from, to, signal] : pending_signals)
                {
                    using enum ModuleType;
                    using enum Signal;
                    using enum ModuleState;

                    // Sometimes we send signals into the void
                    auto it{ subtree.find(to) };
                    if (it != subtree.end())
                    {
                        auto& to_mod{ it->second };
                        switch (to_mod.Type)
                        {
                        case Input:
                            for (auto& out : to_mod.Outputs)
                            {
                                next_signals.push_back({ to, out, signal });
                            }
                            break;
                        case FlipFlop:
                            if (signal == Low)
                            {
                                to_mod.State = to_mod.State == On ? Off : On;
                                const auto out_sig{ to_mod.State == On ? High : Low };
                                for (auto& out : to_mod.Outputs)
                                {
                                    next_signals.push_back({ to, out, out_sig });
                                }
                            }
                            break;
                        case Conjunction:
                        {
                            const auto prev_state{ to_module_state(to_mod) };
                            for (auto [out, last_sig] : std::views::zip(to_mod.Inputs, to_mod.LastSignals))
                            {
                                if (out == from)
                                {
                                    last_sig = signal;
                                }
                            }
                            const auto state{ to_module_state(to_mod) };
                            if (to == output && prev_state != state)
                            {
                                fmt::print("{} from {} to {} @{}\n", output, prev_state == On ? "ON" : "OFF", state == On ? "ON" : "OFF", seen_states.size());
                            }
                            const auto out_sig{ state == On ? Low : High };
                            for (auto& out : to_mod.Outputs)
                            {
                                next_signals.push_back({ to, out, out_sig });
                            }
                            break;
                        }
                        case Output:
                        default:
                            break;
                        }
                    }
                }

                std::swap(pending_signals, next_signals);
            }
        }

        fmt::print("Subtree @{} cycles from {} with cycle length {}\n", root, cycle_start, cycle_len);
        loops.push_back({ cycle_start, cycle_len });
    }

    size_t num_buttons_needed{ loops[0].Length };
    for (auto loop : loops | std::views::drop(1))
    {
        num_buttons_needed = std::lcm(num_buttons_needed, loop.Length);
    }
    fmt::print("The result is: {}", num_buttons_needed);
    return num_buttons_needed != 238420328103151;
}
