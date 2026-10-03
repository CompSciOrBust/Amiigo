#pragma once
#include <string>

namespace Amiigo::Lang {
    void init();
    const std::u32string& get(const char* key);
    std::string getNarrow(const char* key);
    std::u32string utf8ToU32(const std::string& utf8);
    std::string u32ToUtf8(const std::u32string& u32);
}
