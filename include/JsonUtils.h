#pragma once

#include <string>
#include <nlohmann/json.hpp>

using JsonDoc = nlohmann::json;

JsonDoc loadJsonFile(const std::string& path);
JsonDoc parseJsonString(const std::string& str);
void writeJsonFile(const std::string& path, const JsonDoc& doc);

template<typename T>
T readJsonField(const std::string& filePath, const std::string& field, T defaultValue = T{}) {
    JsonDoc j = loadJsonFile(filePath);
    if (j.is_discarded() || !j.contains(field)) return defaultValue;
    return j[field].get<T>();
}
