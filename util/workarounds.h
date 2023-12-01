#pragma once

#if not defined(__cpp_lib_constexpr_charconv) or __cpp_lib_constexpr_charconv < 202207L

#include <string_view>

#include "tokenize_to_types.h"

template<>
constexpr auto ToType<std::uint64_t>(std::string_view type_as_string)
{
    constexpr auto stoi_impl = [](std::string_view str, std::uint64_t value, auto& self) -> std::uint64_t
    {
        constexpr auto is_digit = [](char c) -> bool
        {
            return c <= '9' && c >= '0';
        };

        if (str.empty())
        {
            return value;
        }
        else
        {
            const char front{ str[0] };
            if (is_digit(front))
            {
                return self(str.substr(1), (front - '0') + value * 10, self);
            }
            else
            {
                throw "compile-time-error: not a digit";
            }
        }
    };
    return stoi_impl(type_as_string, 0, stoi_impl);
}

#endif
