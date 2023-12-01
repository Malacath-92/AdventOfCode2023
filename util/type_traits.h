#pragma once

template<class... T>
struct type_list
{
};

template<auto V>
struct constant
{
    inline static constexpr auto value{ V };
};
