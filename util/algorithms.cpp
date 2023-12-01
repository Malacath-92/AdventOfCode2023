#include "algorithms.h"

#include <cctype>

namespace algo
{
std::string to_lower(std::string str)
{
    std::string lower_case{ std::move(str) };
    std::transform(lower_case.begin(), lower_case.end(), lower_case.begin(), [](char c)
                   { return static_cast<char>(std::tolower(c)); });
    return lower_case;
}
std::string to_upper(std::string str)
{
    std::string upper_case{ std::move(str) };
    std::transform(upper_case.begin(), upper_case.end(), upper_case.begin(), [](char c)
                   { return static_cast<char>(std::toupper(c)); });
    return upper_case;
}
} // namespace algo
