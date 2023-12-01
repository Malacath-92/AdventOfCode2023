#pragma once

#include <array>
#include <charconv>
#include <functional>

#include "literal_string.h"
#include "tokenize.h"

template<class T>
constexpr auto ToType(std::string_view type_as_string)
{
    T val{};
    std::from_chars(type_as_string.data(),
                    type_as_string.data() + type_as_string.size(),
                    val);
    return val;
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
    LiteralString Input,
    LiteralString Delimiter = ' ',
    size_t Storage = TokenizeStorageAuto,
    TokenizeBehavior Behavior = TokenizeBehavior::None,
    size_t MaxTokens = std::string_view::npos>
constexpr auto TokenizeToTypes()
{
    using Tokenizer = Tokenize<Delimiter, Storage, Behavior, MaxTokens>;
    // Not random access iterators, so we need to get size like this
    constexpr auto NUM_TOKENS{ []()
                               {
                                   auto tokenizer{ Tokenizer(std::string_view{ Input.str, Input.size }) };
                                   size_t i{};
                                   for ([[maybe_unused]] auto token : tokenizer)
                                   {
                                       ++i;
                                   }
                                   return i;
                               }() };

    std::array<T, NUM_TOKENS> out_array{};

    auto tokenizer{ Tokenizer(std::string_view{ Input.str, Input.size }) };
    size_t i{};
    while (i != NUM_TOKENS)
    {
        out_array[i] = ToType<T>(*tokenizer);
        ++i;
        ++tokenizer;
    }
    return out_array;
}

template<auto Token, LiteralString... Strings>
consteval auto TokenizeToLiteralStringsImpl()
{
    if constexpr (Token == Token.end())
    {
        return LiteralStringList<Strings...>{};
    }
    else
    {
        constexpr auto S{ *Token };
        constexpr auto N{ S.size() + 1 };
        return TokenizeToLiteralStringsImpl<Token.Next(), Strings..., LiteralString{ S.data(), LiteralStringPointerTag<N>{} }>();
    }
}

template<
    LiteralString Input,
    LiteralString Delimiter = ' ',
    TokenizeBehavior Behavior = TokenizeBehavior::None,
    size_t MaxTokens = std::string_view::npos>
consteval auto TokenizeToLiteralStrings()
{
#ifdef __GNUC__
    // Workaround for gcc that does not want to use string views in the tokenizer
    using Tokenizer = Tokenize<Delimiter, Input.size + 1, Behavior, MaxTokens>;
#else
    using Tokenizer = Tokenize<Delimiter, 0, Behavior, MaxTokens>;
#endif

    return TokenizeToLiteralStringsImpl<Tokenizer{ Input }>();
}
