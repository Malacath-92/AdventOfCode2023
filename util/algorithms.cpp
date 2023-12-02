#include "algorithms.h"

#include <cctype>
#include <cstdio>

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

std::string replace_substr(std::string str, std::string_view substr, std::string_view replace)
{
    size_t pos = 0;
    while ((pos = str.find(substr, pos)) != std::string::npos)
    {
        str.replace(pos, substr.length(), replace);
        pos += replace.length();
    }
    return std::move(str);
}

std::string read_whole_file(std::string_view file_path)
{
    FILE* file{ nullptr };
    auto error = fopen_s(&file, std::string{ file_path }.c_str(), "rb");
    if (error == 0 && file != nullptr)
    {
        struct CloseFileOnScopeExit
        {
            ~CloseFileOnScopeExit()
            {
                fclose(File);
            }
            FILE* File;
        };
        auto close_file = CloseFileOnScopeExit{ file };

        fseek(file, 0, SEEK_END);
        const size_t file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        std::string code(file_size, '\0');

        const auto size_read = fread(code.data(), 1, file_size, file);
        if (size_read != file_size)
        {
            code.clear();
            return code;
        }

        algo::erase(code, '\r');
        return code;
    }
    return {};
}
} // namespace algo
