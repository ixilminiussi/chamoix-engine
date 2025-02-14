#ifndef CMX_MISC
#define CMX_MISC

#include <regex>
#include <string>

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

#endif
