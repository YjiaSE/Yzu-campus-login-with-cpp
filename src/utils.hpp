#pragma once

#include <string>

namespace yzu {

std::string trim(const std::string& value);
bool contains(const std::string& haystack, const std::string& needle);
std::string jsonStringField(const std::string& json, const std::string& field);

} // namespace yzu
