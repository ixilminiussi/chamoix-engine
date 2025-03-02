#ifndef CMX_UTILS
#define CMX_UTILS

// std
#include <functional>
#include <regex>
#include <string>

namespace cmx
{

// from: https://stackoverflow.com/a/57595105
template <typename T, typename... Rest> void hashCombine(std::size_t &seed, const T &v, const Rest &...rest)
{
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    (hashCombine(seed, rest), ...);
};

inline std::string incrementNumberInParentheses(const std::string &input)
{
    std::regex pattern(R"(\((\d+)\))"); // Matches "(n)" where n is a number
    std::smatch match;

    if (std::regex_search(input, match, pattern))
    {
        int num = std::stoi(match[1].str()); // Extract number and convert to int
        std::string newStr = std::regex_replace(input, pattern, "(" + std::to_string(num + 1) + ")");
        return newStr;
    }

    return input + "(1)"; // Return original string if no match is found
}

} // namespace cmx

#endif
