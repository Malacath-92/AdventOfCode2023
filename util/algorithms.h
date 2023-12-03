#pragma once

#include "concepts.h"

#include <filesystem>
#include <functional>
#include <numeric>
#include <string_view>
#include <utility>
#include <vector>

namespace algo
{
template<range ContainerT>
constexpr auto max_index(ContainerT&& container)
{
    const auto begin_it = get_begin(container);
    const auto end_it = get_end(container);
    const auto max_it = std::max_element(begin_it, end_it);
    return static_cast<std::size_t>(max_it - begin_it);
}
template<range ContainerT, range_element_invocable<ContainerT> FunT>
constexpr auto max_index(ContainerT&& container, FunT&& fun)
{
    const auto begin_it = get_begin(container);
    const auto end_it = get_end(container);
    auto compare = [fun = make_range_element_invocable<ContainerT>(std::forward<FunT>(fun))](auto& lhs, auto& rhs)
    { return fun(lhs) < fun(rhs); };
    const auto max_it = std::max_element(begin_it, end_it, compare);
    return static_cast<std::size_t>(max_it - begin_it);
}
template<range ContainerT, range_element_addable<ContainerT> ValueT>
constexpr auto accumulate(ContainerT&& container, ValueT&& initial_value = {})
{
    const auto begin_it = get_begin(container);
    const auto end_it = get_end(container);
    return std::accumulate(begin_it, end_it, std::forward<ValueT>(initial_value));
}
template<typename T>
constexpr T&& min(T&& val)
{
    return std::forward<T>(val);
}
template<typename T0, typename T1, typename... Ts>
constexpr auto min(T0&& lhs, T1&& rhs, Ts&&... vs)
{
    return (lhs < rhs)
               ? min(std::forward<T0>(lhs), std::forward<Ts>(vs)...)
               : min(std::forward<T1>(rhs), std::forward<Ts>(vs)...);
}
template<typename T>
constexpr T&& max(T&& val)
{
    return std::forward<T>(val);
}
template<typename T0, typename T1, typename... Ts>
constexpr auto max(T0&& lhs, T1&& rhs, Ts&&... vs)
{
    return (lhs > rhs)
               ? max(std::forward<T0>(lhs), std::forward<Ts>(vs)...)
               : max(std::forward<T1>(rhs), std::forward<Ts>(vs)...);
}

template<range ContainerT, range_element_invocable_r<ContainerT, bool> FunT>
constexpr auto all_of(ContainerT&& container, FunT&& fun)
{
    const auto begin_it = get_begin(container);
    const auto end_it = get_end(container);
    return std::all_of(begin_it, end_it, make_range_element_invocable<ContainerT>(std::forward<FunT>(fun)));
}
template<range ContainerT, range_element_invocable_r<ContainerT, bool> FunT>
constexpr auto any_of(ContainerT&& container, FunT&& fun)
{
    const auto begin_it = get_begin(container);
    const auto end_it = get_end(container);
    return std::any_of(begin_it, end_it, make_range_element_invocable<ContainerT>(std::forward<FunT>(fun)));
}
template<range ContainerT, range_element_invocable_r<ContainerT, bool> FunT>
constexpr auto none_of(ContainerT&& container, FunT&& fun)
{
    const auto begin_it = get_begin(container);
    const auto end_it = get_end(container);
    return std::none_of(begin_it, end_it, make_range_element_invocable<ContainerT>(std::forward<FunT>(fun)));
}

template<range ContainerT, range_element_invocable_r<ContainerT, bool> FunT>
constexpr void erase_if(ContainerT&& container, FunT&& fun)
{
    const auto begin_it = get_begin(container);
    const auto end_it = get_end(container);
    container.erase(std::remove_if(begin_it, end_it, make_range_element_invocable<ContainerT>(std::forward<FunT>(fun))), end_it);
}
template<range ContainerT, range_element_comparable<ContainerT> ValueT>
constexpr void erase(ContainerT&& container, ValueT&& value)
{
    const auto begin_it = get_begin(container);
    const auto end_it = get_end(container);
    container.erase(std::remove(begin_it, end_it, std::forward<ValueT>(value)), end_it);
}
template<class ContainerT, range_element_invocable_r<ContainerT, bool> FunT>
constexpr void erase(ContainerT&& container, FunT&& fun)
{
    erase_if(std::forward<ContainerT>(container), make_range_element_invocable<ContainerT>(std::forward<FunT>(fun)));
}
template<class ContainerT, range_element_invocable<ContainerT> FunT, range_element_invoke_result_comparable<FunT, ContainerT> V>
constexpr void erase(ContainerT&& container, FunT&& fun, V&& val)
{
    erase_if(std::forward<ContainerT>(container),
             [fun = make_range_element_invocable<ContainerT>(std::forward<FunT>(fun)), val = std::forward<V>(val)](auto& element)
             { return fun(element) == val; });
}

template<range ContainerT, range_element_invocable_r<ContainerT, bool> FunT>
constexpr auto find_if(ContainerT&& container, FunT&& fun) -> range_value_t<ContainerT>*
{
    const auto begin_it = get_begin(container);
    const auto end_it = get_end(container);
    auto found_it = std::find_if(begin_it, end_it, make_range_element_invocable<ContainerT>(std::forward<FunT>(fun)));
    if (found_it != end_it)
    {
        return &*found_it;
    }
    return nullptr;
}
template<range ContainerT, range_element_comparable<ContainerT> ValueT>
constexpr auto find(ContainerT&& container, ValueT&& value) -> range_value_t<ContainerT>*
{
    const auto begin_it = get_begin(container);
    const auto end_it = get_end(container);
    auto found_it = std::find(begin_it, end_it, std::forward<ValueT>(value));
    if (found_it != end_it)
    {
        return &*found_it;
    }
    return nullptr;
}
template<range ContainerT, range_element_invocable_r<ContainerT, bool> FunT>
constexpr auto find(ContainerT&& container, FunT&& fun) -> range_value_t<ContainerT>*
{
    return find_if(std::forward<ContainerT>(container), make_range_element_invocable<ContainerT>(std::forward<FunT>(fun)));
}
template<range ContainerT, range_element_invocable<ContainerT> FunT, range_element_invoke_result_comparable<FunT, ContainerT> V>
constexpr auto find(ContainerT&& container, FunT&& fun, V&& val) -> range_value_t<ContainerT>*
{
    return find_if(std::forward<ContainerT>(container),
                   [fun = make_range_element_invocable<ContainerT>(std::forward<FunT>(fun)), val = std::forward<V>(val)](auto& element)
                   { return fun(element) == val; });
}

template<range ContainerT, range_element_invocable_r<ContainerT, bool> FunT>
constexpr auto contains_if(ContainerT&& container, FunT&& fun)
{
    return find_if(std::forward<ContainerT>(container), make_range_element_invocable<ContainerT>(std::forward<FunT>(fun))) != nullptr;
}
template<range ContainerT, range_element_comparable<ContainerT> ValueT>
constexpr auto contains(ContainerT&& container, ValueT&& value)
{
    return find(std::forward<ContainerT>(container), std::forward<ValueT>(value)) != nullptr;
}
template<range ContainerT, range_element_invocable_r<ContainerT, bool> FunT>
constexpr auto contains(ContainerT&& container, FunT&& fun)
{
    return contains_if(std::forward<ContainerT>(container), make_range_element_invocable<ContainerT>(std::forward<FunT>(fun)));
}
template<range ContainerT, range_element_invocable<ContainerT> FunT, range_element_invoke_result_comparable<FunT, ContainerT> V>
constexpr auto contains(ContainerT&& container, FunT&& fun, V&& val)
{
    return contains_if(std::forward<ContainerT>(container),
                       [fun = make_range_element_invocable<ContainerT>(std::forward<FunT>(fun)), val = std::forward<V>(val)](auto& element)
                       { return fun(element) == val; });
}

template<class ContainerT, class NewValueT>
struct rebind_value;
template<class OldValueT, class... ArgsT, template<class...> class ContainerT, class NewValueT>
struct rebind_value<ContainerT<OldValueT, ArgsT...>, NewValueT>
{
    using type = ContainerT<NewValueT, typename rebind_value<ArgsT, NewValueT>::type...>;
};
template<range ContainerT, class NewValueT>
using rebind_value_t = typename rebind_value<ContainerT, NewValueT>::type;

template<resizable_range ContainerT,
         class FunT>
constexpr auto transformed(ContainerT&& container, FunT&& fun)
{
    using NewValueT = std::decay_t<std::invoke_result_t<FunT, range_element_t<ContainerT>>>;
    using OutContainerT = rebind_value_t<std::decay_t<ContainerT>, std::decay_t<NewValueT>>;

    auto in_begin{ get_begin(container) };
    auto in_end{ get_end(container) };
    const size_t size{ static_cast<size_t>(std::distance(in_begin, in_end)) };

    OutContainerT out{};
    out.resize(size);
    std::transform(in_begin, in_end, get_begin(out), std::forward<FunT>(fun));
    return out;
}
template<class OutContainerT,
         range ContainerT,
         range_element_invocable_r<ContainerT, range_element_t<OutContainerT>> FunT>
constexpr auto transformed(ContainerT&& container, FunT&& fun)
{
    auto in_begin{ get_begin(container) };
    auto in_end{ get_end(container) };
    const size_t size{ static_cast<size_t>(std::distance(in_begin, in_end)) };

    OutContainerT out{};
    out.resize(size);
    std::transform(in_begin, in_end, get_begin(out), std::forward<FunT>(fun));
    return out;
}
template<range ContainerT,
         range_element_invocable<ContainerT> FunT,
         std::invocable<const ContainerT&> MakeOutContainerFunT>
constexpr auto transformed(ContainerT&& container, FunT&& fun, MakeOutContainerFunT&& make_out_container)
{
    auto out{ std::invoke(make_out_container, container) };
    std::transform(get_begin(container), get_end(container), get_begin(out), std::forward<FunT>(fun));
    return out;
}

template<range ContainerT>
constexpr void reverse(ContainerT&& container)
{
    std::reverse(get_begin(container), get_end(container));
}
template<range ContainerT>
constexpr auto reverted(ContainerT&& container)
{
    auto copy{ std::forward<ContainerT>(container) };
    algo::reverse(copy);
    return copy;
}

template<range ContainerT>
constexpr void sort(ContainerT&& container)
{
    const auto begin_it = get_begin(container);
    const auto end_it = get_end(container);
    std::sort(begin_it, end_it);
}
template<range ContainerT, range_element_invocable_r<ContainerT, bool> FunT>
constexpr void sort(ContainerT&& container, FunT&& fun)
{
    const auto begin_it = get_begin(container);
    const auto end_it = get_end(container);
    std::sort(begin_it, end_it, make_range_element_invocable<ContainerT>(std::forward<FunT>(fun)));
}
template<range ContainerT, range_element_invocable<ContainerT> FunT>
constexpr void sort_by_member(ContainerT&& container, FunT&& fun)
{
    sort(std::forward<ContainerT>(container),
         [fun = make_range_element_invocable<ContainerT>(std::forward<FunT>(fun))](auto& lhs, auto& rhs)
         { return fun(lhs) < fun(rhs); });
}

template<range ContainerT>
constexpr auto sorted(ContainerT&& container)
{
    auto copy{ std::forward<ContainerT>(container) };
    algo::sort(copy);
    return copy;
}
template<range ContainerT, range_element_invocable_r<ContainerT, bool> FunT>
constexpr auto sort(ContainerT&& container, FunT&& fun)
{
    auto copy{ std::forward<ContainerT>(container) };
    algo::sort(copy, std::forward<FunT>(fun));
    return copy;
}
template<range ContainerT, range_element_invocable<ContainerT> FunT>
constexpr auto sort_by_member(ContainerT&& container, FunT&& fun)
{
    auto copy{ std::forward<ContainerT>(container) };
    algo::sort(copy, std::forward<FunT>(fun));
    return copy;
}

template<range ContainerT>
constexpr auto is_sub_set(ContainerT&& sub_set, ContainerT&& container)
{
    return all_of(std::forward<ContainerT>(sub_set),
                  [container = std::forward<ContainerT>(container)](const auto& val)
                  { return contains(container, val); });
}

template<range ContainerT, range_element_invocable_r<ContainerT, bool> FunT>
constexpr auto count_if(ContainerT&& container, FunT&& fun)
{
    return std::count_if(get_begin(container), get_end(container), make_range_element_invocable<ContainerT>(std::forward<FunT>(fun)));
}
template<range ContainerT, range_element_comparable<ContainerT> ValueT>
constexpr auto count(ContainerT&& container, ValueT&& value)
{
    return std::count(get_begin(container), get_end(container), std::forward<ValueT>(value));
}
template<range ContainerT, range_element_invocable_r<ContainerT, bool> FunT>
constexpr auto count(ContainerT&& container, FunT&& fun)
{
    return count_if(std::forward<ContainerT>(container), make_range_element_invocable<ContainerT>(std::forward<FunT>(fun)));
}
template<range ContainerT, range_element_invocable<ContainerT> FunT, range_element_invoke_result_comparable<FunT, ContainerT> V>
constexpr auto count(ContainerT&& container, FunT&& fun, V&& val)
{
    return count_if(std::forward<ContainerT>(container),
                    [fun = make_range_element_invocable<ContainerT>(std::forward<FunT>(fun)), val = std::forward<V>(val)](auto& element)
                    { return fun(element) == val; });
}

template<range ContainerT, range_element_convertible<ContainerT> ValueT>
constexpr auto replace(ContainerT&& container, ValueT&& from, ValueT&& to)
{
    std::replace(get_begin(container), get_end(container), std::forward<ValueT>(from), std::forward<ValueT>(to));
    return std::forward<ContainerT>(container);
}
template<range ContainerT, range_element_invocable<ContainerT> FunT, range_element_convertible<ContainerT> ValueT>
constexpr auto replace(ContainerT&& container, FunT&& from, ValueT&& to)
{
    std::replace_if(get_begin(container), get_end(container), std::forward<FunT>(from), std::forward<ValueT>(to));
    return std::forward<ContainerT>(container);
}

template<class T>
constexpr T stoi(std::string_view str)
{
    constexpr auto stoi_impl = [](std::string_view str, T value, auto& self) -> T
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
    return stoi_impl(str, 0, stoi_impl);
}

template<range ContainerT,
         class ValueT = range_element_t<ContainerT>,
         class CharT = typename ValueT::value_type>
requires one_of<range_element_t<ContainerT>, std::string_view, std::string, const char*>
constexpr std::string join(const ContainerT& elements, const CharT* delimiter)
{
    std::basic_ostringstream<CharT> stream;
    auto begin = std::begin(elements);
    auto end = std::end(elements);

    if (begin != end)
    {
        std::copy(begin, std::prev(end), std::ostream_iterator<ValueT>{ stream, delimiter });
        begin = std::prev(end);
    }

    if (begin != end)
    {
        stream << *begin;
    }

    return stream.str();
}

std::string path_string(const std::filesystem::path& path);
bool is_same_path(const std::filesystem::path& lhs, const std::filesystem::path& rhs);
bool is_sub_path(const std::filesystem::path& path, const std::filesystem::path& base);
bool is_end_of_path(const std::filesystem::path& path, const std::filesystem::path& base);
std::filesystem::path strip_end_of_path(const std::filesystem::path& path, const std::filesystem::path& base);

std::string trim(std::string str);
std::string trim(std::string str, char to_trim);

template<class FunT>
constexpr std::string_view trim(std::string_view str, FunT&& fun)
{
    auto left = str.begin();
    for (;; ++left)
    {
        if (left == str.end())
        {
            return {};
        }

        if (!fun(*left))
        {
            break;
        }
    }

    auto right = str.end() - 1;
    // clang-format off
    for (; right > left && fun(*right); --right);
    // clang-format on

    return std::string_view{ &*left, static_cast<size_t>(std::distance(left, right) + 1) };
}
constexpr std::string_view trim(std::string_view str)
{
    constexpr auto isspace = [](char c)
    {
        constexpr char spaces[]{
            ' ',
            '\f',
            '\n',
            '\r',
            '\t',
            '\v',
        };
        return contains(spaces, c);
    };
    return trim(str, isspace);
}
constexpr std::string_view trim(std::string_view str, char to_trim)
{
    return trim(str, [=](char c)
                { return c == to_trim; });
}

constexpr bool case_insensitive_equal(std::string_view lhs, std::string_view rhs)
{
    struct case_insensitive_char_traits : public std::char_traits<char>
    {
        static bool eq(char c1, char c2)
        {
            return toupper(c1) == toupper(c2);
        }
        static bool ne(char c1, char c2)
        {
            return toupper(c1) != toupper(c2);
        }
        static bool lt(char c1, char c2)
        {
            return toupper(c1) < toupper(c2);
        }
        static int compare(const char* s1, const char* s2, size_t n)
        {
            while (n-- != 0)
            {
                if (toupper(*s1) < toupper(*s2))
                    return -1;
                if (toupper(*s1) > toupper(*s2))
                    return 1;
                ++s1;
                ++s2;
            }
            return 0;
        }
        static const char* find(const char* s, int n, char a)
        {
            while (n-- > 0 && toupper(*s) != toupper(a))
            {
                ++s;
            }
            return s;
        }
    };
    using case_insensitive_string_view = std::basic_string_view<char, case_insensitive_char_traits>;
    return case_insensitive_string_view{ lhs.data(), lhs.size() } == case_insensitive_string_view{ rhs.data(), rhs.size() };
}
template<std::size_t N>
constexpr bool case_insensitive_equal(char (&lhs)[N], std::string_view rhs)
{
    return case_insensitive_equal(std::string_view{ lhs, N }, rhs);
}
template<std::size_t N>
constexpr bool case_insensitive_equal(std::string_view lhs, char (&rhs)[N])
{
    return case_insensitive_equal(lhs, std::string_view{ rhs, N });
}
template<std::size_t N, std::size_t M>
constexpr bool case_insensitive_equal(char (&lhs)[N], char (&rhs)[M])
{
    return case_insensitive_equal(std::string_view{ lhs, N }, std::string_view{ rhs, M });
}

std::string to_lower(std::string str);
std::string to_upper(std::string str);

std::string replace_substr(std::string str, std::string_view substr, std::string_view replace);

std::string read_whole_file(std::string_view file_path);

// Intentionally copies args, require std::reference_wrapper if people want references
// https://github.com/lefticus/tools/blob/main/include/lefticus/tools/curry.hpp
constexpr decltype(auto) curry(auto f, auto... ps)
{
    if constexpr (requires { std::invoke(f, ps...); })
    {
        return std::invoke(f, ps...);
    }
    else
    {
        return [f, ps...](auto... qs) -> decltype(auto)
        { return curry(f, ps..., qs...); };
    }
}
// https://www.youtube.com/watch?v=s2Kqcn5e73c
constexpr decltype(auto) bind_front(auto f, auto... ps)
{
    return [f, ps...](auto... qs) -> decltype(auto)
    { return std::invoke(f, ps..., qs...); };
}
constexpr decltype(auto) bind_back(auto f, auto... ps)
{
    return [f, ps...](auto... qs) -> decltype(auto)
    { return std::invoke(f, qs..., ps...); };
}
} // namespace algo
