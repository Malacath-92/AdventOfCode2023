#pragma once

#include <array>
#include <charconv>
#include <functional>

#include "literal_string.h"
#include "tokenize.h"

template<class T>
constexpr auto ToType(std::string_view)
{
    return T{};
}
template<>
constexpr auto ToType<char>(std::string_view type_as_string)
{
    return type_as_string.front();
}
template<>
constexpr auto ToType<std::string_view>(std::string_view type_as_string)
{
    return type_as_string;
}
// Fully specialize this for the types you like, or move into a tmpl-struct to
// be able to partially specialize

template<
    class T,
    LiteralString Delimiter = ' ',
    size_t Storage = TokenizeStorageAuto,
    TokenizeBehavior Behavior = TokenizeBehavior::TrimWhitespace | TokenizeBehavior::SkipEmpty,
    size_t MaxTokens = std::string_view::npos>
constexpr auto TokenizeToTypes(std::string_view input)
{
    using Tokenizer = Tokenize<Delimiter, Storage, Behavior, MaxTokens>;

    std::vector<T> out_array{};
    for (auto token : Tokenizer(std::string_view{ input }))
    {
        out_array.push_back(ToType<T>(token));
    }
    return out_array;
}
