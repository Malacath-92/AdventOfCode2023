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

    std::unordered_map modules{ file_data | lines | to_string_views | to_modules | to_unordered_map };
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

    static constexpr size_t c_NumberPresses{ 1000 };

    std::vector<int64_t> low_signals_per_press;
    std::vector<int64_t> high_signals_per_press;

    for (size_t i = 0; i < c_NumberPresses; i++)
    {
        int64_t low_signals{ 0 };
        int64_t high_signals{ 0 };
        const auto count_signals = [&](Signal sig, auto num)
        {
            (sig == Signal::Low ? low_signals : high_signals) += static_cast<int64_t>(num);
        };

        PendingSignalList pending_signals{ { "button", "broadcaster", Signal::Low } };
        count_signals(Signal::Low, 1);

        while (!pending_signals.empty())
        {
            PendingSignalList next_signals{};
            for (auto [from, to, signal] : pending_signals)
            {
                using enum ModuleType;
                using enum Signal;
                using enum ModuleState;

                // Sometimes we send signals into the void
                auto it{ modules.find(to) };
                if (it != modules.end())
                {
                    auto& to_mod{ it->second };
                    switch (to_mod.Type)
                    {
                    case Input:
                        for (auto& out : to_mod.Outputs)
                        {
                            next_signals.push_back({ to, out, signal });
                        }
                        count_signals(signal, to_mod.Outputs.size());
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
                            count_signals(out_sig, to_mod.Outputs.size());
                        }
                        break;
                    case Conjunction:
                    {
                        for (auto [out, last_sig] : std::views::zip(to_mod.Inputs, to_mod.LastSignals))
                        {
                            if (out == from)
                            {
                                last_sig = signal;
                            }
                        }
                        const auto state{ to_module_state(to_mod) };
                        const auto out_sig{ state == On ? Low : High };
                        for (auto& out : to_mod.Outputs)
                        {
                            next_signals.push_back({ to, out, out_sig });
                        }
                        count_signals(out_sig, to_mod.Outputs.size());
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

        low_signals_per_press.push_back(low_signals);
        high_signals_per_press.push_back(high_signals);
    }

    const size_t total_low_signals{ algo::accumulate(low_signals_per_press, size_t{ 0 }) };
    const size_t total_high_signals{ algo::accumulate(high_signals_per_press, size_t{ 0 }) };

    const size_t num_signals_sent{ total_low_signals * total_high_signals };
    fmt::print("The result is: {}", num_signals_sent);
    return num_signals_sent != 898557000;
}
