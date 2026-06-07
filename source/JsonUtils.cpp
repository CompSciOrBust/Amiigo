#include <JsonUtils.h>

#include <fstream>
#include <iomanip>

JsonDoc loadJsonFile(const std::string& path) {
    std::ifstream fileStream(path);
    std::string content((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
    return JsonDoc::parse(content, nullptr, false);
}

JsonDoc parseJsonString(const std::string& str) {
    return JsonDoc::parse(str, nullptr, false);
}

void writeJsonFile(const std::string& path, const JsonDoc& doc) {
    std::ofstream fileStream(path);
    fileStream << std::setw(4) << doc << std::endl;
}
