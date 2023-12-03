#pragma once

#include <concepts>
#include <functional>
#include <memory>
#include <type_traits>

namespace algo
{
template<class T, class... Ts>
concept one_of = (std::same_as<T, Ts> || ...);

template<class T, class U>
concept is_comparable =
    requires(T&& lhs, T&& rhs) {
        lhs == rhs;
        rhs == lhs;
    };

template<class T>
concept std_range =
    requires(T&& cont) {
        std::begin(cont);
        std::end(cont);
    };
template<class T>
concept not_std_range =
    !requires(T&& cont) {
        std::begin(cont);
        std::end(cont);
    };
template<class T>
concept adl_range =
    requires(T&& cont) {
        begin(cont);
        end(cont);
    };
template<class T>
concept not_adl_range =
    !requires(T&& cont) {
        begin(cont);
        end(cont);
    };
template<class T>
concept member_range =
    requires(T&& cont) {
        cont.begin();
        cont.end();
    };

template<class ContainerT>
requires member_range<ContainerT> && not_adl_range<ContainerT> && not_std_range<ContainerT>
constexpr auto get_begin(ContainerT&& cont)
{
    return cont.begin();
}
template<class ContainerT>
requires member_range<ContainerT> && not_adl_range<ContainerT> && not_std_range<ContainerT>
constexpr auto get_end(ContainerT&& cont)
{
    return cont.end();
}
template<class ContainerT>
requires adl_range<ContainerT> && not_std_range<ContainerT>
constexpr auto get_begin(ContainerT&& cont)
{
    return begin(cont);
}
template<class ContainerT>
requires adl_range<ContainerT> && not_std_range<ContainerT>
constexpr auto get_end(ContainerT&& cont)
{
    return end(cont);
}
template<class ContainerT>
requires std_range<ContainerT>
constexpr auto get_begin(ContainerT&& cont)
{
    return std::begin(cont);
}
template<class ContainerT>
requires std_range<ContainerT>
constexpr auto get_end(ContainerT&& cont)
{
    return std::end(cont);
}

template<class T>
concept range =
    requires(T&& cont) {
        get_begin(cont);
        get_end(cont);
    };

template<class T>
concept resizable_range =
    range<T> && requires(T&&) {
        std::declval<std::remove_const_t<std::decay_t<T>>>().resize(size_t{});
    };

template<class T>
requires range<T>
using range_element_t = std::decay_t<decltype(*get_begin(std::declval<T>()))>;
template<class T>
requires range<T>
using range_value_t = std::remove_reference_t<decltype(*get_begin(std::declval<T>()))>;

// clang-format off
template<class ValueT, class ContainerT>
concept range_element_addable =
    requires(ContainerT&& cont, ValueT&& val) {
        *get_begin(std::forward<ContainerT>(cont)) + val;
    };
template<class FunT, class ContainerT>
concept range_element_invocable_plain =
    requires(ContainerT&& cont, FunT&& fun) {
        std::forward<FunT>(fun)(*get_begin(std::forward<ContainerT>(cont)));
    };
template<class FunT, class ContainerT>
concept range_element_invocable_mem_fun =
    requires(ContainerT&& cont, FunT&& fun) {
        std::mem_fn(std::forward<FunT>(fun))(*get_begin(std::forward<ContainerT>(cont)));
    };
template<class FunT, class ContainerT>
concept range_element_invocable =
    range_element_invocable_plain<FunT, ContainerT> || range_element_invocable_mem_fun<FunT, ContainerT>;
template<class FunT, class ContainerT, class RetT>
concept range_element_invocable_plain_r =
    requires(ContainerT&& cont, FunT&& fun) {
        {
            std::forward<FunT>(fun)(*get_begin(std::forward<ContainerT>(cont)))
        } -> std::same_as<RetT>;
    };
template<class FunT, class ContainerT, class RetT>
concept range_element_invocable_mem_fun_r =
    requires(ContainerT&& cont, FunT&& fun) {
        {
            std::mem_fn(std::forward<FunT>(fun))(*get_begin(std::forward<ContainerT>(cont)))
        } -> std::same_as<RetT>;
    };
template<class FunT, class ContainerT, class RetT>
concept range_element_invocable_r =
    range_element_invocable_plain_r<FunT, ContainerT, RetT> || range_element_invocable_mem_fun_r<FunT, ContainerT, RetT>;

template<class ValueT, class ContainerT>
concept range_element_convertible = std::convertible_to<ValueT, range_value_t<ContainerT>>;
template<class ValueT, class ContainerT>
concept range_element_comparable = std::equality_comparable_with<ValueT, range_value_t<ContainerT>>;
template<class FunT, class ContainerT>
using range_element_invoke_result_t =
    std::invoke_result_t<decltype(make_range_element_invocable<ContainerT>(std::declval<FunT&&>())), range_element_t<ContainerT>>;
template<class ValueT, class FunT, class ContainerT>
concept range_element_invoke_result_comparable = std::equality_comparable_with<ValueT, range_element_invoke_result_t<FunT, ContainerT>>;
// clang-format on

template<class ContainerT, range_element_invocable_plain<ContainerT> FunT>
constexpr auto make_range_element_invocable(FunT&& fun)
{
    return std::forward<FunT>(fun);
}
template<class ContainerT, range_element_invocable_mem_fun<ContainerT> FunT>
constexpr auto make_range_element_invocable(FunT&& fun)
{
    return std::mem_fn(std::forward<FunT>(fun));
}

template<class T, class U>
requires range<T>
struct range_contains
{
    using value_t = std::decay_t<range_element_t<T>>;
    using expec_t = std::decay_t<U>;
    // clang-format off
    inline static constexpr bool value =
           std::is_same_v<value_t, expec_t>
        || std::is_same_v<value_t, expec_t*>
        || std::is_same_v<value_t, std::unique_ptr<expec_t>>
        || std::is_same_v<value_t, std::shared_ptr<expec_t>>;
    // clang-format on
};
template<class T, class U>
requires range<T>
inline constexpr bool range_contains_v = range_contains<T, U>::value;
} // namespace algo
