#include "utils.hpp"

#include <cctype>
#include <regex>

namespace yzu {

std::string trim(const std::string& value)
{
    std::size_t begin = 0;
    while (begin < value.size() && std::isspace(static_cast<unsigned char>(value[begin]))) {
        ++begin;
    }

    std::size_t end = value.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }

    return value.substr(begin, end - begin);
}

bool contains(const std::string& haystack, const std::string& needle)
{
    return haystack.find(needle) != std::string::npos;
}

std::string jsonStringField(const std::string& json, const std::string& field)
{
    const std::regex pattern("\"" + field + R"json("\s*:\s*"((?:[^"\\]|\\.)*)")json");
    std::smatch match;
    if (!std::regex_search(json, match, pattern) || match.size() < 2) {
        return "";
    }

    std::string value = match[1].str();
    value = std::regex_replace(value, std::regex(R"(\\")"), "\"");
    value = std::regex_replace(value, std::regex(R"(\\\\)"), "\\");
    return value;
}

} // namespace yzu
