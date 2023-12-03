#pragma once

#include <string_view>
#include <vector>

#include "algorithms.h"
#include "literal_string.h"

enum class TokenizeBehavior
{
    None = 0,
    TrimWhitespace = 1 << 0,
    SkipEmpty = 1 << 1,
    AnyOfDelimiter = 1 << 2,
};
constexpr TokenizeBehavior operator|(TokenizeBehavior rhs, TokenizeBehavior lhs)
{
    return (TokenizeBehavior)((int)lhs | (int)rhs);
}
constexpr bool operator&(TokenizeBehavior rhs, TokenizeBehavior lhs)
{
    return ((int)lhs & (int)rhs) != 0;
}

inline constexpr size_t TokenizeStorageAuto{ 0 };

struct TokenizeStringView
{
    explicit constexpr TokenizeStringView(std::string_view str)
        : data{ str.data() }
        , size{ str.size() }
    {
    }
    template<size_t N>
    explicit constexpr TokenizeStringView(const LiteralString<N>& source)
        : TokenizeStringView{ source.std_view() }
    {
    }

    constexpr auto std_view() const
    {
        return std::string_view{ data, size };
    }

    constexpr bool operator==(const TokenizeStringView& rhs) const = default;
    constexpr bool operator!=(const TokenizeStringView& rhs) const = default;

    const char* data;
    size_t size;
};

template<size_t N>
struct TokenizeStringStorageImpl
{
    using type = LiteralString<N>;
};
template<>
struct TokenizeStringStorageImpl<0>
{
    using type = TokenizeStringView;
};
template<size_t N>
using TokenizeStringStorage = typename TokenizeStringStorageImpl<N>::type;

template<LiteralString Delimiter,
         size_t StorageSize = TokenizeStorageAuto,
         TokenizeBehavior Behavior = TokenizeBehavior::None,
         size_t MaxTokens = std::string_view::npos>
class Tokenize
{
  public:
    constexpr Tokenize() = default;
    constexpr Tokenize(const Tokenize&) = default;
    constexpr Tokenize(Tokenize&&) noexcept = default;
    constexpr Tokenize& operator=(const Tokenize&) = default;
    constexpr Tokenize& operator=(Tokenize&&) noexcept = default;
    constexpr ~Tokenize() = default;

    explicit constexpr Tokenize(std::nullptr_t)
    {
    }
    explicit constexpr Tokenize(const char* source)
        : m_Source{ std::string_view{ source } }
    {
        GetNext();
    }
    explicit constexpr Tokenize(std::string_view source)
        : m_Source{ source }
    {
        GetNext();
    }
    template<size_t N>
    explicit constexpr Tokenize(const LiteralString<N>& source)
        : m_Source{ source }
    {
        GetNext();
    }

    constexpr bool operator==(const Tokenize& rhs) const = default;
    constexpr bool operator!=(const Tokenize& rhs) const = default;

    constexpr auto begin()
    {
        return *this;
    }
    constexpr auto end()
    {
        return Tokenize{ m_Source, end_tag_t{} };
    }
    constexpr auto begin() const
    {
        return *this;
    }
    constexpr auto end() const
    {
        return Tokenize{ m_Source, end_tag_t{} };
    }
    constexpr auto cbegin() const
    {
        return *this;
    }
    constexpr auto cend() const
    {
        return Tokenize{ m_Source, end_tag_t{} };
    }

    constexpr auto operator*() const
    {
        const size_t pos{
            m_Position == 0 || (Behavior & TokenizeBehavior::AnyOfDelimiter)
                ? m_Position
                : m_Position + Delimiter.size - 1
        };
        const std::string_view res = [](std::string_view token)
        {
            if constexpr (Behavior & TokenizeBehavior::TrimWhitespace)
            {
                return algo::trim(token);
            }
            else
            {
                return token;
            }
        }(m_Source.std_view().substr(pos, m_Next - pos));
        return res;
    }

    constexpr decltype(auto) operator++()
    {
        if (!Advance())
        {
            *this = end();
        }
        return *this;
    }
    constexpr auto operator++(int)
    {
        auto copy = *this;
        ++(*this);
        return copy;
    }

    constexpr auto Next() const
    {
        auto copy = *this;
        return ++copy;
    }

    // Exposed internals for use as a literal type
    using StringStorage = TokenizeStringStorage<StorageSize>;
    StringStorage m_Source;
    size_t m_Position{ 0 };
    size_t m_Next{ 0 };
    size_t m_NumTokens{ 0 };

  private:
    struct end_tag_t
    {
    };
    constexpr Tokenize(StringStorage source, end_tag_t)
        : m_Source{ source }
        , m_Position{ source.size }
        , m_Next{ source.size }
    {
    }

    constexpr void GetNext()
    {
        if constexpr (MaxTokens != std::string_view::npos)
        {
            if (m_NumTokens == MaxTokens)
            {
                m_Next = m_Source.size;
                return;
            }
        }

        m_Next = m_Position;
        if constexpr (Behavior & TokenizeBehavior::AnyOfDelimiter)
        {
            while (m_Next < m_Source.size && !algo::contains(Delimiter.str, m_Source.std_view()[m_Next]))
            {
                ++m_Next;
            }
        }
        else
        {
            while (m_Next < m_Source.size && !m_Source.std_view().substr(m_Next).starts_with(Delimiter.str))
            {
                ++m_Next;
            }
        }

        if constexpr (Behavior & TokenizeBehavior::SkipEmpty)
        {
            while (m_Next == m_Position && m_Position < m_Source.size)
            {
                m_Position = m_Next + 1;
                GetNext();
            }

            if (m_Position != m_Next)
            {
                ++m_NumTokens;
            }
        }
        else
        {
            ++m_NumTokens;
        }
    }

    constexpr bool Advance()
    {
        m_Position = m_Next + 1;
        GetNext();
        return m_Position < m_Source.size;
    }
};

namespace algo
{
template<LiteralString Delimeter, class OutputT = std::string_view>
constexpr std::vector<OutputT> split(std::string_view str)
{
    std::vector<OutputT> sub_strings;
    for (auto&& sub_string : Tokenize<Delimeter>{ str })
    {
        sub_strings.push_back(OutputT{ sub_string });
    }
    return sub_strings;
}
} // namespace algo
