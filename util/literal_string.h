#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <string_view>

template<size_t N>
struct LiteralStringPointerTag
{
};

template<size_t N>
struct LiteralString
{
    consteval LiteralString(const char (&in_str)[N])
    {
        std::copy(std::begin(in_str), std::end(in_str), std::begin(str));
    }
    consteval LiteralString(const char* in_str, LiteralStringPointerTag<N>)
    {
        std::copy(in_str, in_str + N, std::begin(str));
    }

    constexpr auto std_view() const
    {
        return std::string_view{ str, size };
    }

    constexpr bool operator==(const LiteralString& rhs) const = default;
    constexpr bool operator!=(const LiteralString& rhs) const = default;

    char str[N]{};
    inline static constexpr std::size_t size{ N - 1 };
};
template<>
struct LiteralString<2>
{
    consteval LiteralString(const char (&in_str)[2])
    {
        std::copy(std::begin(in_str), std::end(in_str), std::begin(str));
    }
    consteval LiteralString(const char* in_str, LiteralStringPointerTag<2>)
    {
        std::copy(in_str, in_str + 2, std::begin(str));
    }
    consteval LiteralString(char c)
    {
        str[0] = c;
        str[1] = '\0';
    }

    constexpr auto std_view() const
    {
        return std::string_view{ str, size };
    }

    constexpr bool operator==(const LiteralString& rhs) const = default;
    constexpr bool operator!=(const LiteralString& rhs) const = default;

    char str[2]{};
    inline static constexpr std::size_t size{ 1 };
};

template<size_t N = 2>
LiteralString(char) -> LiteralString<2>;
template<size_t N>
LiteralString(const char*, LiteralStringPointerTag<N>) -> LiteralString<N>;

template<LiteralString... Strings>
struct LiteralStringList
{
    static constexpr auto size()
    {
        return sizeof...(Strings);
    }
    template<std::size_t I>
    requires(I < sizeof...(Strings))
    static constexpr auto get()
    {
        return std::get<I>(std::tuple{ Strings... });
    }
    static constexpr auto std_array()
    {
        return std::array{ Strings.std_view()... };
    }
};
